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

    std::unordered_set<std::string> candidateFieldList = GetCandidateFieldsList(recordDeclaration, result.Context);

    bool haveConstructors = HandleConstructors(recordDeclaration,
                                               candidateFieldList,
                                               result.Context);

    if (!haveConstructors)
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

        candidates.insert(fieldDeclaration->getName().str());
    }

    return candidates;
}

bool UninitializedFieldRule::HandleConstructors(const RecordDecl* recordDeclaration,
                                                const std::unordered_set<std::string>& candidateFieldList,
                                                ASTContext* context)
{
    bool haveConstructors = false;

    for (auto it = recordDeclaration->decls_begin();
         it != recordDeclaration->decls_end();
         ++it)
    {
        if (!CXXConstructorDecl::classof(*it))
            continue;

        std::unordered_set<std::string> constructorCandidateFieldList = candidateFieldList;

        const CXXConstructorDecl* constructorDeclaration = static_cast<const CXXConstructorDecl*>(*it);
        bool validConstructor = HandleConstructorDeclaration(constructorDeclaration, constructorCandidateFieldList);
        if (!validConstructor)
            continue;

        haveConstructors = true;

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

    return haveConstructors;
}

bool UninitializedFieldRule::HandleConstructorDeclaration(const clang::CXXConstructorDecl* constructorDeclaration,
                                                          std::unordered_set<std::string>& candidateFieldList)
{
    // constructor without body is a declaration, not a definition
    if (!constructorDeclaration->hasBody())
        return false;

    const DeclContext* declarationContext = constructorDeclaration->getDeclContext();
    if (declarationContext == nullptr ||
        ! declarationContext->isRecord())
    {
        return false;
    }

    HandleInitializationList(constructorDeclaration, candidateFieldList);

    HandleConstructorBody(constructorDeclaration, candidateFieldList);

    return true;
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
