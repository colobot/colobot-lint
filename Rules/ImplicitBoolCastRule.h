#pragma once

#include "Rules/ASTCallbackRule.h"

#include <clang/ASTMatchers/ASTMatchFinder.h>

class ImplicitBoolCastRule : public ASTCallbackRule,
                             public clang::ast_matchers::MatchFinder::MatchCallback
{
public:
    ImplicitBoolCastRule(Context& context);

    void RegisterASTMatcherCallback(clang::ast_matchers::MatchFinder& finder) override;

    void run(const clang::ast_matchers::MatchFinder::MatchResult& result) override;

    static const char* GetName() { return "ImplicitBoolCastRule"; }

private:
    clang::ast_matchers::StatementMatcher m_matcher;
};
