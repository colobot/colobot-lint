#pragma once

struct Context;

namespace clang
{

class CompilerInstance;

namespace ast_matchers
{
class MatchFinder;
} // namespace ast_matchers

} // namespace clang


class Rule
{
public:
    Rule(Context& context)
        : m_context(context)
    {}

    virtual ~Rule()
    {}

    virtual void RegisterASTMatcherCallback(clang::ast_matchers::MatchFinder& /*finder*/)
    {}

    virtual void RegisterPreProcessorCallbacks(clang::CompilerInstance& /*ci*/)
    {}

protected:
    Context& m_context;
};
