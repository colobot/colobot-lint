#pragma once

#include "DirectASTConsumerRule.h"

#include "clang/AST/RecursiveASTVisitor.h"

#include <unordered_set>
#include <set>

class BlockPlacementRule : public DirectASTConsumerRule,
                           public clang::RecursiveASTVisitor<BlockPlacementRule>
{
public:
    BlockPlacementRule(Context& context);

    void HandleTranslationUnit(clang::ASTContext &context) override;

    bool VisitDecl(clang::Decl* declaration);
    bool VisitStmt(clang::Stmt* statement,
                   clang::SourceLocation* overridenLocStartForFunctionBodies = nullptr);

private:
    bool IsDeclarationLineConfigurationAllowed(
        int declarationLineNumber,
        int bodyStartLineNumber,
        int bodyEndLineNumber) const;
    bool IsOpeningBracePlacedCorrectly(const clang::SourceLocation& locStart,
                                       const clang::SourceLocation& locEnd);
    bool IsClosingBracePlacedCorrectly(const clang::SourceLocation& locStart,
                                       const clang::SourceLocation& locEnd);

    void ReportViolationWithDeclaration(clang::Decl* declaration);
    void ReportViolationWithStatement(clang::Stmt* statement);

    clang::ASTContext* m_astContext;
    // forbidden lines are lines where we can't have other statements or declarations
    std::unordered_set<int> m_forbiddenLineNumbers;
    // where problems have already been reported; this is to avoid doubled errors
    std::unordered_set<int> m_reportedLineNumbers;
    std::set<clang::SourceLocation> m_visitedFunctionBodies;
};
