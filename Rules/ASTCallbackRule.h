#pragma once

#include "Rule.h"

#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Frontend/CompilerInstance.h"

class ASTCallbackRule : public Rule
{
public:
    ASTCallbackRule(Context& context)
        : Rule(context)
    {}

    virtual void RegisterASTMatcherCallback(clang::ast_matchers::MatchFinder& finder)
    {}

    virtual void RegisterPreProcessorCallbacks(clang::CompilerInstance& compiler)
    {}
};
