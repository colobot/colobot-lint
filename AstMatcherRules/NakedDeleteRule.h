#pragma once

#include "Rule.h"

class NakedDeleteRule : public Rule
{
public:
    NakedDeleteRule(clang::ast_matchers::MatchFinder& finder,
                    OutputPrinter& printer);

    void run(const clang::ast_matchers::MatchFinder::MatchResult& Result) override;

private:
    clang::ast_matchers::StatementMatcher m_matcher;
};
