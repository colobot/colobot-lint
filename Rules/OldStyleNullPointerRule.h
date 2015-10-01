#pragma once

#include "Rules/Rule.h"

#include <clang/ASTMatchers/ASTMatchFinder.h>

class OldStyleNullPointerRule : public Rule,
                                public clang::ast_matchers::MatchFinder::MatchCallback
{
public:
    OldStyleNullPointerRule(Context& context);

    void RegisterASTMatcherCallback(clang::ast_matchers::MatchFinder& finder) override;

    void run(const clang::ast_matchers::MatchFinder::MatchResult& result) override;

    static const char* GetName() { return "OldStyleNullPointerRule"; }

private:
    void handleZeroLiteralNullExpression(const clang::Expr* zeroLiteralNullExpression,
                                         clang::SourceManager& sourceManager);
    void handleGnuNullExpression(const clang::GNUNullExpr* gnuNullExpression,
                                 const clang::Expr* parentExpression,
                                 clang::SourceManager& sourceManager);
};
