#include "UninitializedFieldRule.hpp"

#include "../Common/Context.h"
#include "../Common/OutputPrinter.h"
#include "../Common/SourceLocationHelper.h"
#include "../Common/RegexConsts.h"

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

    for (auto it = recordDeclaration->decls_begin();
         it != recordDeclaration->decls_end();
         ++it)
    {
        if (!CXXConstructorDecl::classof(*it))
            continue;

        const CXXConstructorDecl* constructorDeclaration = static_cast<const CXXConstructorDecl*>(*it);
        if (constructorDeclaration->isCopyOrMoveConstructor() &&
            constructorDeclaration->isImplicit())
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

    for (auto it = recordDeclaration->decls_begin();
         it != recordDeclaration->decls_end();
         ++it)
    {
        if (!FieldDecl::classof(*it))
            continue;

        const FieldDecl* fieldDeclaration = static_cast<const FieldDecl*>(*it);
        if (fieldDeclaration->hasInClassInitializer())
            continue;

        QualType type = fieldDeclaration->getType();
        if (! type.isPODType(*context))
        {
            continue;
        }

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
    for (auto it = recordDeclaration->decls_begin();
         it != recordDeclaration->decls_end();
         ++it)
    {
        if (!CXXConstructorDecl::classof(*it))
            continue;

        const CXXConstructorDecl* constructorDeclaration = static_cast<const CXXConstructorDecl*>(*it);
        if (constructorDeclaration->isCopyOrMoveConstructor() &&
            constructorDeclaration->isImplicit())
            continue;

        std::unordered_set<std::string> constructorCandidateFieldList = candidateFieldList;
        HandleConstructorDeclaration(constructorDeclaration, constructorCandidateFieldList);

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

void UninitializedFieldRule::HandleConstructorDeclaration(const clang::CXXConstructorDecl* constructorDeclaration,
                                                          std::unordered_set<std::string>& candidateFieldList)
{
    // constructor without body is a declaration, not a definition
    if (!constructorDeclaration->hasBody())
    {
        std::cerr << "No body!" << std::endl;
        return;
    }

    const DeclContext* declarationContext = constructorDeclaration->getDeclContext();
    if (declarationContext == nullptr ||
        ! declarationContext->isRecord())
    {
        return;
    }

    HandleInitializationList(constructorDeclaration, candidateFieldList);

    HandleConstructorBody(constructorDeclaration, candidateFieldList);
}

void UninitializedFieldRule::HandleInitializationList(const CXXConstructorDecl* constructorDeclaration,
                                                      std::unordered_set<std::string>& candidateFieldList)
{
    for (auto it = constructorDeclaration->init_begin();
         it != constructorDeclaration->init_end();
         ++it)
    {
        FieldDecl* member = (*it)->getMember();
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
        for (auto it = compountStatement->body_begin();
             it != compountStatement->body_end();
             ++it)
        {
            if (BinaryOperator::classof(*it))
            {
                const BinaryOperator* binaryOperator = static_cast<const BinaryOperator*>(*it);
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
    if (leftHandSide != nullptr &&
        MemberExpr::classof(leftHandSide))
    {
        const MemberExpr* memberExpr = static_cast<const MemberExpr*>(leftHandSide);
        const Expr* baseExpr = memberExpr->getBase();
        const ValueDecl* memberDecl = memberExpr->getMemberDecl();
        if (baseExpr != nullptr &&
            CXXThisExpr::classof(memberExpr->getBase()) &&
            memberDecl != nullptr)
        {
            candidateFieldList.erase(memberDecl->getName().str());
        }
    }
}
