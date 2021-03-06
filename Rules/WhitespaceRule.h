#pragma once

#include <clang/ASTMatchers/ASTMatchFinder.h>

#include "Rules/Rule.h"

class WhitespaceRule : public Rule,
                       public clang::ast_matchers::MatchFinder::MatchCallback
{
public:
    WhitespaceRule(Context& context);

    void RegisterASTMatcherCallback(clang::ast_matchers::MatchFinder& finder) override;

    void run(const clang::ast_matchers::MatchFinder::MatchResult& result) override;

    static const char* GetName() { return "WhitespaceRule"; }
};
