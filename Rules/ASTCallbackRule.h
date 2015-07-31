#pragma once

#include "Rules/Rule.h"

namespace clang
{

class CompilerInstance;

namespace ast_matchers
{
class MatchFinder;
} // namespace ast_matchers

} // namespace clang

class ASTCallbackRule : public Rule
{
public:
    ASTCallbackRule(Context& context)
        : Rule(context)
    {}

    virtual void RegisterASTMatcherCallback(clang::ast_matchers::MatchFinder& /*finder*/)
    {}

    virtual void RegisterPreProcessorCallbacks(clang::CompilerInstance& /*ci*/)
    {}
};
