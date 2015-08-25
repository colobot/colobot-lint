#pragma once

#include "Rules/ASTCallbackRule.h"

#include <clang/ASTMatchers/ASTMatchFinder.h>

class NakedDeleteRule : public ASTCallbackRule,
                        public clang::ast_matchers::MatchFinder::MatchCallback
{
public:
    NakedDeleteRule(Context& context);

    void RegisterASTMatcherCallback(clang::ast_matchers::MatchFinder& finder) override;

    void run(const clang::ast_matchers::MatchFinder::MatchResult& result) override;

    static const char* GetName() { return "NakedDeleteRule"; }
};
