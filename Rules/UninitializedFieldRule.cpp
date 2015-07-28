#include "UninitializedFieldRule.h"

#include "../Common/Context.h"
#include "../Common/OutputPrinter.h"
#include "../Common/SourceLocationHelper.h"

#include <clang/AST/Decl.h>
#include <clang/AST/Stmt.h>

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

    SourceLocation location = recordDeclaration->getLocation();
    if (! m_context.sourceLocationHelper.IsLocationOfInterest(location, result.Context->getSourceManager()))
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

    std::unordered_set<std::string> candidateFieldList = GetCandidateFieldsList(recordDeclaration, result.Context);

    if (constructorStatus == ConstructorStatus::NoConstructors)
    {
        for (const auto& field : candidateFieldList)
        {
            std::string which = recordDeclaration->isClass() ? "Class" : "Struct";

            m_context.printer.PrintRuleViolation(
                "uninitialized field",
                Severity::Error,
                which + " '" + recordDeclaration->getName().str() + "' field '" + field + "'" + " remains uninitialized",
                location,
                result.Context->getSourceManager());
        }
    }
    else
    {
        HandleConstructors(recordDeclaration, candidateFieldList, result.Context);
    }
}

UninitializedFieldRule::ConstructorStatus UninitializedFieldRule::CheckConstructorStatus(const RecordDecl* recordDeclaration)
{
    bool haveConstructorsWithBody = false;

    for (const Decl* decl : recordDeclaration->decls())
    {
        if (!CXXConstructorDecl::classof(decl))
            continue;

        const CXXConstructorDecl* constructorDeclaration = static_cast<const CXXConstructorDecl*>(decl);
        if (constructorDeclaration->isImplicit())
            continue;

        if (! constructorDeclaration->hasBody())
            return ConstructorStatus::SomeConstructorsNotDefined;

        haveConstructorsWithBody = true;
    }

    return haveConstructorsWithBody ? ConstructorStatus::DefinedConstructors : ConstructorStatus::NoConstructors;
}

std::unordered_set<std::string> UninitializedFieldRule::GetCandidateFieldsList(const RecordDecl* recordDeclaration,
                                                                               ASTContext* context)
{
    std::unordered_set<std::string> candidates;

    for (const Decl* decl : recordDeclaration->decls())
    {
        if (!FieldDecl::classof(decl))
            continue;

        const FieldDecl* fieldDeclaration = static_cast<const FieldDecl*>(decl);
        if (fieldDeclaration->hasInClassInitializer())
            continue;

        QualType type = fieldDeclaration->getType();
        if (! type.isPODType(*context))
            continue;

        if (fieldDeclaration->isAnonymousStructOrUnion())
            continue;

        candidates.insert(fieldDeclaration->getName().str());
    }

    return candidates;
}

void UninitializedFieldRule::HandleConstructors(const RecordDecl* recordDeclaration,
                                                const std::unordered_set<std::string>& candidateFieldList,
                                                ASTContext* context)
{
    for (const Decl* decl : recordDeclaration->decls())
    {
        if (!CXXConstructorDecl::classof(decl))
            continue;

        const CXXConstructorDecl* constructorDeclaration = static_cast<const CXXConstructorDecl*>(decl);

        if (constructorDeclaration->isImplicit() ||
            ! constructorDeclaration->isThisDeclarationADefinition())
        {
            continue;
        }

        std::unordered_set<std::string> constructorCandidateFieldList = candidateFieldList;
        HandleConstructorInitializationList(constructorDeclaration, constructorCandidateFieldList);
        HandleConstructorBody(constructorDeclaration, constructorCandidateFieldList);

        for (const auto& field : constructorCandidateFieldList)
        {
            std::string which = recordDeclaration->isClass() ? "Class" : "Struct";

            m_context.printer.PrintRuleViolation(
                "uninitialized field",
                Severity::Error,
                which + " '" + recordDeclaration->getName().str() + "' field '" + field + "'" + " remains uninitialized in constructor",
                constructorDeclaration->getLocation(),
                context->getSourceManager());
        }
    }
}

void UninitializedFieldRule::HandleConstructorInitializationList(const CXXConstructorDecl* constructorDeclaration,
                                                                 std::unordered_set<std::string>& candidateFieldList)
{
    for (CXXCtorInitializer* init : constructorDeclaration->inits())
    {
        FieldDecl* member = init->getMember();
        if (member != nullptr)
        {
            candidateFieldList.erase(member->getName().str());
        }
    }
}

void UninitializedFieldRule::HandleConstructorBody(const CXXConstructorDecl* constructorDeclaration,
                                                   std::unordered_set<std::string>& candidateFieldList)
{
    const Stmt* constructorBody = constructorDeclaration->getBody();
    if (constructorBody != nullptr &&
        CompoundStmt::classof(constructorBody))
    {
        const CompoundStmt* compountStatement = static_cast<const CompoundStmt*>(constructorBody);
        for (Stmt* statement : compountStatement->body())
        {
            if (BinaryOperator::classof(statement))
            {
                const BinaryOperator* binaryOperator = static_cast<const BinaryOperator*>(statement);
                if (binaryOperator->isAssignmentOp())
                {
                    HandleAssignStatement(binaryOperator, candidateFieldList);
                }
            }
        }
    }
}

void UninitializedFieldRule::HandleAssignStatement(const BinaryOperator* assignStatement,
                                                   std::unordered_set<std::string>& candidateFieldList)
{
    const Stmt* leftHandSide = assignStatement->getLHS();
    if (leftHandSide == nullptr)
        return;

    if (!MemberExpr::classof(leftHandSide))
        return;

    const MemberExpr* memberExpr = static_cast<const MemberExpr*>(leftHandSide);
    const Expr* baseExpr = memberExpr->getBase();
    if (baseExpr == nullptr)
        return;

    if (! CXXThisExpr::classof(memberExpr->getBase()))
        return;

    const ValueDecl* memberDecl = memberExpr->getMemberDecl();
    if (memberDecl == nullptr)
        return;

    candidateFieldList.erase(memberDecl->getName().str());
}
