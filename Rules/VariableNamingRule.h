#pragma once

#include "ASTCallbackRule.h"

#include <clang/ASTMatchers/ASTMatchFinder.h>

#include <boost/regex.hpp>

class VariableNamingRule : public ASTCallbackRule,
                           public clang::ast_matchers::MatchFinder::MatchCallback
{
public:
    VariableNamingRule(Context& context);

    void RegisterASTMatcherCallback(clang::ast_matchers::MatchFinder& finder) override;

    void run(const clang::ast_matchers::MatchFinder::MatchResult& result) override;

    static const char* GetName() { return "VariableNamingRule"; }

private:
    clang::ast_matchers::DeclarationMatcher m_matcher;
    boost::regex m_localVariableNamePattern;
    boost::regex m_nonConstGlobalVariableNamePattern;
    boost::regex m_constGlobalVariableNamePattern;
    boost::regex m_deprecatedVariableNamePattern;
};
