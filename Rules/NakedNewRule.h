#pragma once

#include "ASTCallbackRule.h"

class NakedNewRule : public ASTCallbackRule,
                     public clang::ast_matchers::MatchFinder::MatchCallback
{
public:
    NakedNewRule(Context& context);

    void RegisterASTMatcherCallback(clang::ast_matchers::MatchFinder& finder) override;

    void run(const clang::ast_matchers::MatchFinder::MatchResult& Result) override;

private:
    clang::ast_matchers::StatementMatcher m_matcher;
};
