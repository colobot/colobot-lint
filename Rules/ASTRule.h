#pragma once

#include "Rule.h"

#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Frontend/CompilerInstance.h"

class ASTRule : public Rule
{
public:
    ASTRule(Context& context)
        : Rule(context)
    {}

    virtual void RegisterASTMatcherCallback(clang::ast_matchers::MatchFinder& finder)
    {}

    virtual void RegisterPreProcessorCallbacks(clang::CompilerInstance& compiler)
    {}
};
