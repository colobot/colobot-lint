#pragma once

#include "Rules/Rule.h"

#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/AST/RecursiveASTVisitor.h>

#include <llvm/ADT/DenseSet.h>

class BlockPlacementRule : public Rule,
                           public clang::ast_matchers::MatchFinder::MatchCallback,
                           public clang::RecursiveASTVisitor<BlockPlacementRule>
{
public:
    BlockPlacementRule(Context& context);

    void RegisterASTMatcherCallback(clang::ast_matchers::MatchFinder& finder) override;
    void run(const clang::ast_matchers::MatchFinder::MatchResult& result) override;

    bool TraverseDecl(clang::Decl* declaration);

    bool VisitDecl(clang::Decl* declaration);
    bool VisitStmt(clang::Stmt* statement);

    static const char* GetName() { return "BlockPlacementRule"; }

private:
    bool IsDeclarationOpeningBracePlacedCorrectly(const clang::SourceLocation& locStart,
                                                  const clang::SourceLocation& locEnd);
    bool IsStatementOpeningBracePlacedCorrectly(const clang::SourceLocation& openingBraceLocation);
    bool IsClosingBracePlacedCorrectly(const clang::SourceLocation& locStart,
                                       const clang::SourceLocation& locEnd);

    enum class ViolationType
    {
        OpeningBrace,
        ClosingBrace
    };

    void ReportViolation(const clang::SourceLocation& location, ViolationType type);

    clang::ASTContext* m_astContext = nullptr;
    clang::FileID m_mainFileID;
    // forbidden lines are where we know we have closing braces
    // of previously visited statements or declarations
    llvm::DenseSet<int> m_forbiddenLineNumbers;
    // where problems have already been reported; this is to avoid doubled errors
    llvm::DenseSet<int> m_reportedLineNumbers;
};
