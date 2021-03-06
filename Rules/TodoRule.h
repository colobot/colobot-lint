#pragma once

#include "Rules/Rule.h"

#include <clang/Lex/Preprocessor.h>

#include <boost/regex.hpp>

class TodoRule : public Rule,
                 public clang::CommentHandler
{
public:
    TodoRule(Context& context);

    void RegisterPreProcessorCallbacks(clang::CompilerInstance& compiler) override;

    bool HandleComment(clang::Preprocessor& pp, clang::SourceRange range) override;

    static const char* GetName() { return "TodoRule"; }

private:
    boost::regex m_todoPattern;
};
