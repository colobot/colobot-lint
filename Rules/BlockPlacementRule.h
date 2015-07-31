#pragma once

#include "Rules/DirectASTConsumerRule.h"

#include <clang/AST/RecursiveASTVisitor.h>

#include <unordered_set>

class BlockPlacementRule : public DirectASTConsumerRule,
                           public clang::RecursiveASTVisitor<BlockPlacementRule>
{
public:
    BlockPlacementRule(Context& context);

    void HandleTranslationUnit(clang::ASTContext &context) override;

    bool VisitDecl(clang::Decl* declaration);
    bool VisitStmt(clang::Stmt* statement);

    static const char* GetName() { return "BlockPlacementRule"; }

private:
    bool IsDeclarationOpeningBracePlacedCorrectly(const clang::SourceLocation& locStart,
                                                  const clang::SourceLocation& locEnd);
    bool IsStatementOpeningBracePlacedCorrectly(const clang::SourceLocation& parentStartLocation,
                                                const clang::SourceLocation& braceLocation);
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
