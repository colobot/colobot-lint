#pragma once

#include "Rule.h"

class NakedDeleteRule : public Rule,
                        public clang::ast_matchers::MatchFinder::MatchCallback
{
public:
    NakedDeleteRule(Context& context);

    void RegisterASTMatcherCallback(clang::ast_matchers::MatchFinder& finder) override;

    void run(const clang::ast_matchers::MatchFinder::MatchResult& result) override;

private:
    clang::ast_matchers::StatementMatcher m_matcher;
};
