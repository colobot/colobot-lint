#pragma once

#include "ASTRule.h"

#include "clang/Lex/Preprocessor.h"

#include <regex>

class TodoRule : public ASTRule,
                 public clang::CommentHandler
{
public:
    TodoRule(Context& context);

    void RegisterPreProcessorCallbacks(clang::CompilerInstance& compiler) override;

    bool HandleComment(clang::Preprocessor& pp, clang::SourceRange range) override;

private:
    std::vector<std::string> SplitLines(const std::string& text);

    std::regex m_todoPattern;
};
