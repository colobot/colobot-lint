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
      m_recordDeclarationMatcher(recordDecl().bind("recordDecl")),
      m_constructorDeclarationMatcher(constructorDecl().bind("constructorDecl"))
{}

void UninitializedFieldRule::RegisterASTMatcherCallback(MatchFinder& finder)
{
    finder.addMatcher(m_recordDeclarationMatcher, this);
    finder.addMatcher(m_constructorDeclarationMatcher, this);
}

void UninitializedFieldRule::run(const MatchFinder::MatchResult& result)
{
    const RecordDecl* recordDeclaration = result.Nodes.getNodeAs<RecordDecl>("recordDecl");
    if (recordDeclaration != nullptr)
        return HandleRecordDeclaration(recordDeclaration, result.Context, nullptr);

    const CXXConstructorDecl* constructorDeclaration = result.Nodes.getNodeAs<CXXConstructorDecl>("constructorDecl");
    if (constructorDeclaration != nullptr)
        return HandleConstructorDeclaration(constructorDeclaration, result.Context);
}

void UninitializedFieldRule::HandleRecordDeclaration(const RecordDecl* recordDeclaration,
                                                     ASTContext* context,
                                                     const CXXConstructorDecl* constructorDeclarationToCheck)
{
    SourceManager& sourceManager = context->getSourceManager();

    SourceLocation location = recordDeclaration->getLocation();
    if (! m_context.sourceLocationHelper.IsLocationOfInterest(GetName(), location, sourceManager))
        return;

    if (recordDeclaration->isUnion())
        return;

    if (AreThereInterestingConstructorDeclarations(recordDeclaration))
        return; // will be handled through HandleConstructorDeclaration()

    // Since no constructors are defined, all candidates are treated as uninitialized
    StringRefSet candidateFieldList = GetCandidateFieldsList(recordDeclaration, context);
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

void UninitializedFieldRule::HandleConstructorDeclaration(const CXXConstructorDecl* constructorDeclaration,
                                                          ASTContext* context)
{
    SourceManager& sourceManager = context->getSourceManager();

    SourceLocation location = constructorDeclaration->getLocation();
    if (! m_context.sourceLocationHelper.IsLocationOfInterest(GetName(), location, sourceManager))
        return;

    if (constructorDeclaration->isImplicit() ||
        constructorDeclaration->isDeleted() ||
        ! constructorDeclaration->isThisDeclarationADefinition())
    {
        return;
    }

    const DeclContext* declContext = constructorDeclaration->getDeclContext();
    if (declContext == nullptr &&
        !declContext->isRecord())
    {
        return;
    }

    const RecordDecl* recordDeclaration = static_cast<const RecordDecl*>(declContext);

    if (recordDeclaration->isUnion())
        return;

    StringRefSet candidateFieldList = GetCandidateFieldsList(recordDeclaration, context);
    CheckInitializationsInInitializationList(constructorDeclaration, candidateFieldList);
    CheckInitializationsInConstructorBody(constructorDeclaration, candidateFieldList);

    for (const auto& field : candidateFieldList)
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

bool UninitializedFieldRule::AreThereInterestingConstructorDeclarations(const RecordDecl* recordDeclaration)
{
    for (const Decl* decl : recordDeclaration->decls())
    {
        const CXXConstructorDecl* constructorDeclaration = classof_cast<const CXXConstructorDecl>(decl);
        if (constructorDeclaration == nullptr ||
            constructorDeclaration->isImplicit() ||
            constructorDeclaration->isDeleted())
        {
            continue;
        }

        return true;
    }

    return false;
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

void UninitializedFieldRule::CheckInitializationsInInitializationList(const CXXConstructorDecl* constructorDeclaration,
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

void UninitializedFieldRule::CheckInitializationsInConstructorBody(const CXXConstructorDecl* constructorDeclaration,
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
            CheckInitializationsInAssignStatement(binaryOperator, candidateFieldList);
        }
    }
}

void UninitializedFieldRule::CheckInitializationsInAssignStatement(const BinaryOperator* assignStatement,
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
