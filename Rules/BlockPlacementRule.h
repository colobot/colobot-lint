#pragma once

#include "DirectASTConsumerRule.h"

#include <clang/AST/RecursiveASTVisitor.h>

#include <unordered_set>
#include <set>

class BlockPlacementRule : public DirectASTConsumerRule,
                           public clang::RecursiveASTVisitor<BlockPlacementRule>
{
public:
    BlockPlacementRule(Context& context);

    void HandleTranslationUnit(clang::ASTContext &context) override;

    bool VisitDecl(clang::Decl* declaration);
    bool VisitStmt(clang::Stmt* statement);

private:
    bool IsOpeningBracePlacedCorrectly(const clang::SourceLocation& locStart,
                                       const clang::SourceLocation& locEnd);
    bool IsClosingBracePlacedCorrectly(const clang::SourceLocation& locStart,
                                       const clang::SourceLocation& locEnd);

    enum class ViolationType
    {
        OpeningBrace,
        ClosingBrace
    };

    void ReportViolation(const clang::SourceLocation& location, ViolationType type);

    clang::ASTContext* m_astContext;
    // forbidden lines are where we know we have closing braces
    // of previously visited statements or declarations
    std::unordered_set<int> m_forbiddenLineNumbers;
    // where problems have already been reported; this is to avoid doubled errors
    std::unordered_set<int> m_reportedLineNumbers;
};
