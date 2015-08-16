#pragma once

#include "Generators/Generator.h"

#include <clang/ASTMatchers/ASTMatchFinder.h>

class DependencyGraphGenerator : public Generator,
                                 public clang::ast_matchers::MatchFinder::MatchCallback
{
public:
    DependencyGraphGenerator(Context& context);

    void RegisterASTMatcherCallback(clang::ast_matchers::MatchFinder& finder) override;

    void run(const clang::ast_matchers::MatchFinder::MatchResult& result) override;

    static const char* GetName() { return "DependencyGraph"; }

private:
    void HandleRecordDeclaration(const clang::CXXRecordDecl* recordDecl,
                                 clang::SourceManager& sourceManager);

    void HandleMemberCallExpression(const clang::CXXMemberCallExpr* memberCallExpr,
                                    const clang::RecordDecl* callerRecordDecl,
                                    clang::SourceManager& sourceManager);

private:
    clang::ast_matchers::DeclarationMatcher m_recordDeclMatcher;
    clang::ast_matchers::StatementMatcher m_memberCallExprMatcher;
};
