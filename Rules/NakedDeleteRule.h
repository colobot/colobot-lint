#pragma once

#include "Rule.h"

#include "clang/ASTMatchers/ASTMatchFinder.h"

#include <memory>

class NakedDeleteRule : public Rule
{
public:
    NakedDeleteRule(clang::ast_matchers::MatchFinder& finder,
                    OutputPrinter& printer);

private:
    clang::ast_matchers::StatementMatcher m_matcher;
    std::unique_ptr<clang::ast_matchers::MatchFinder::MatchCallback> m_callback;
    class Callback;
};
