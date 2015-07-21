#pragma once

#include "ASTCallbackRule.h"

#include <clang/Lex/Preprocessor.h>

#include <boost/regex.hpp>

class TodoRule : public ASTCallbackRule,
                 public clang::CommentHandler
{
public:
    TodoRule(Context& context);

    void RegisterPreProcessorCallbacks(clang::CompilerInstance& compiler) override;

    bool HandleComment(clang::Preprocessor& pp, clang::SourceRange range) override;

private:
    std::vector<std::string> SplitLines(const std::string& text);

    boost::regex m_todoPattern;
};
