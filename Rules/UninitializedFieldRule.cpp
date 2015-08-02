#include "Rules/UninitializedFieldRule.h"

#include "Common/ClassofCast.h"
#include "Common/Context.h"
#include "Common/OutputPrinter.h"
#include "Common/SourceLocationHelper.h"

#include <clang/AST/Decl.h>
#include <clang/AST/Stmt.h>

#include <boost/format.hpp>

using namespace clang;
using namespace clang::ast_matchers;

UninitializedFieldRule::UninitializedFieldRule(Context& context)
    : ASTCallbackRule(context),
      m_matcher(recordDecl().bind("recordDecl"))
{}

void UninitializedFieldRule::RegisterASTMatcherCallback(MatchFinder& finder)
{
    finder.addMatcher(m_matcher, this);
}

void UninitializedFieldRule::run(const MatchFinder::MatchResult& result)
{
    const RecordDecl* recordDeclaration = result.Nodes.getNodeAs<RecordDecl>("recordDecl");
    if (recordDeclaration == nullptr)
        return;

    SourceManager& sourceManager = result.Context->getSourceManager();

    SourceLocation location = recordDeclaration->getLocation();
    if (! m_context.sourceLocationHelper.IsLocationOfInterest(GetName(), location, sourceManager))
        return;

    if (recordDeclaration->isUnion())
        return;

    ConstructorStatus constructorStatus = CheckConstructorStatus(recordDeclaration);

    // Special case when can't see definition of constructors in header file
    if (m_context.areWeInFakeHeaderSourceFile &&
        constructorStatus == ConstructorStatus::SomeConstructorsNotDefined)
    {
        return;
    }

    StringRefSet candidateFieldList = GetCandidateFieldsList(recordDeclaration, result.Context);

    if (constructorStatus == ConstructorStatus::NoConstructors)
    {
        for (const auto& field : candidateFieldList)
        {
            std::string which = recordDeclaration->isClass() ? "Class" : "Struct";

            m_context.printer.PrintRuleViolation(
                "uninitialized field",
                Severity::Error,
                boost::str(boost::format("%s '%s' field '%s' remains uninitialized")
                    % which
                    % recordDeclaration->getName().str()
                    % field.str()),
                location,
                sourceManager);
        }
    }
    else
    {
        HandleConstructors(recordDeclaration, candidateFieldList, sourceManager);
    }
}

UninitializedFieldRule::ConstructorStatus UninitializedFieldRule::CheckConstructorStatus(const RecordDecl* recordDeclaration)
{
    bool haveConstructorsWithBody = false;

    for (const Decl* decl : recordDeclaration->decls())
    {
        const CXXConstructorDecl* constructorDeclaration = classof_cast<const CXXConstructorDecl>(decl);
        if (constructorDeclaration == nullptr ||
            constructorDeclaration->isImplicit())
        {
            continue;
        }

        if (! constructorDeclaration->hasBody())
            return ConstructorStatus::SomeConstructorsNotDefined;

        haveConstructorsWithBody = true;
    }

    return haveConstructorsWithBody ? ConstructorStatus::DefinedConstructors : ConstructorStatus::NoConstructors;
}

UninitializedFieldRule::StringRefSet UninitializedFieldRule::GetCandidateFieldsList(const RecordDecl* recordDeclaration,
                                                                                    ASTContext* context)
{
    StringRefSet candidates;

    for (const Decl* decl : recordDeclaration->decls())
    {
        const FieldDecl* fieldDeclaration = classof_cast<const FieldDecl>(decl);
        if (fieldDeclaration == nullptr ||
            fieldDeclaration->hasInClassInitializer())
            continue;

        QualType type = fieldDeclaration->getType();
        if (! type.isPODType(*context))
            continue;

        if (fieldDeclaration->isAnonymousStructOrUnion())
            continue;

        candidates.insert(fieldDeclaration->getName());
    }

    return candidates;
}

void UninitializedFieldRule::HandleConstructors(const RecordDecl* recordDeclaration,
                                                const UninitializedFieldRule::StringRefSet& candidateFieldList,
                                                SourceManager& sourceManager)
{
    for (const Decl* decl : recordDeclaration->decls())
    {
        const CXXConstructorDecl* constructorDeclaration = classof_cast<const CXXConstructorDecl>(decl);


        if (constructorDeclaration == nullptr ||
            constructorDeclaration->isImplicit())
        {
            continue;
        }

        if (!constructorDeclaration->isThisDeclarationADefinition())
        {
            bool definitionFound = false;

            for (const FunctionDecl* redecl : constructorDeclaration->redecls())
            {
                const CXXConstructorDecl* constructorRedeclaration = classof_cast<const CXXConstructorDecl>(redecl);
                if (constructorRedeclaration != nullptr &&
                    redecl->isThisDeclarationADefinition())
                {
                    constructorDeclaration = constructorRedeclaration;
                    definitionFound = true;
                    break;
                }
            }

            if (!definitionFound)
                continue;
        }

        auto constructorCandidateFieldList = candidateFieldList;
        HandleConstructorInitializationList(constructorDeclaration, constructorCandidateFieldList);
        HandleConstructorBody(constructorDeclaration, constructorCandidateFieldList);

        for (const auto& field : constructorCandidateFieldList)
        {
            std::string which = recordDeclaration->isClass() ? "Class" : "Struct";

            m_context.printer.PrintRuleViolation(
                "uninitialized field",
                Severity::Error,
                boost::str(boost::format("%s '%s' field '%s' remains uninitialized in constructor")
                    % which
                    % recordDeclaration->getName().str()
                    % field.str()),
                constructorDeclaration->getLocation(),
                sourceManager);
        }
    }
}

void UninitializedFieldRule::HandleConstructorInitializationList(const CXXConstructorDecl* constructorDeclaration,
                                                                 UninitializedFieldRule::StringRefSet& candidateFieldList)
{
    for (CXXCtorInitializer* init : constructorDeclaration->inits())
    {
        FieldDecl* member = init->getMember();
        if (member != nullptr)
        {
            candidateFieldList.erase(member->getName());
        }
    }
}

void UninitializedFieldRule::HandleConstructorBody(const CXXConstructorDecl* constructorDeclaration,
                                                   UninitializedFieldRule::StringRefSet& candidateFieldList)
{
    const CompoundStmt* compountStatement = classof_cast<const CompoundStmt>(constructorDeclaration->getBody());
    if (compountStatement == nullptr)
        return;

    for (Stmt* statement : compountStatement->body())
    {
        const BinaryOperator* binaryOperator = classof_cast<const BinaryOperator>(statement);
        if (binaryOperator != nullptr &&
            binaryOperator->isAssignmentOp())
        {
            HandleAssignStatement(binaryOperator, candidateFieldList);
        }
    }
}

void UninitializedFieldRule::HandleAssignStatement(const BinaryOperator* assignStatement,
                                                   UninitializedFieldRule::StringRefSet& candidateFieldList)
{
    const MemberExpr* memberExpr = classof_cast<const MemberExpr>(assignStatement->getLHS());
    if (memberExpr == nullptr)
        return;

    const CXXThisExpr* thisExpr = classof_cast<const CXXThisExpr>(memberExpr->getBase());
    if (thisExpr == nullptr)
        return;

    const ValueDecl* memberDecl = memberExpr->getMemberDecl();
    if (memberDecl == nullptr)
        return;

    candidateFieldList.erase(memberDecl->getName());
}
