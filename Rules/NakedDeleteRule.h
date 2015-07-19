#pragma once

#include "ASTRule.h"

class NakedDeleteRule : public ASTRule,
                        public clang::ast_matchers::MatchFinder::MatchCallback
{
public:
    NakedDeleteRule(Context& context);

    void RegisterASTMatcherCallback(clang::ast_matchers::MatchFinder& finder) override;

    void run(const clang::ast_matchers::MatchFinder::MatchResult& result) override;

private:
    clang::ast_matchers::StatementMatcher m_matcher;
};
