#pragma once

#include "../Common/Context.h"

#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Frontend/CompilerInstance.h"

class Rule
{
public:
    Rule(Context& context)
        : m_context(context)
    {}

    virtual ~Rule()
    {}

    virtual void RegisterASTMatcherCallback(clang::ast_matchers::MatchFinder& finder)
    {}

    virtual void RegisterPreProcessorCallbacks(clang::CompilerInstance &compiler)
    {}

protected:
    Context& m_context;
};
