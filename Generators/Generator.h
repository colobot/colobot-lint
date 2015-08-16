#pragma once

struct Context;

namespace clang
{

namespace ast_matchers
{
class MatchFinder;
} // namespace ast_matchers

} // namespace clang

class Generator
{
public:
    Generator(Context& context)
        : m_context(context)
    {}

    virtual ~Generator()
    {}

    virtual void RegisterASTMatcherCallback(clang::ast_matchers::MatchFinder& /*finder*/)
    {}

protected:
    Context& m_context;
};
