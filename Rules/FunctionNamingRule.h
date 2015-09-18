#pragma once

#include "Rules/Rule.h"

#include <clang/ASTMatchers/ASTMatchFinder.h>

#include <unordered_set>
#include <boost/regex.hpp>

class FunctionNamingRule : public Rule,
                           public clang::ast_matchers::MatchFinder::MatchCallback
{
public:
    FunctionNamingRule(Context& context);

    void RegisterASTMatcherCallback(clang::ast_matchers::MatchFinder& finder) override;

    void run(const clang::ast_matchers::MatchFinder::MatchResult& result) override;

    static const char* GetName() { return "FunctionNamingRule"; }

private:
    void HandleDeclaration(const char* type,
                           const clang::FunctionDecl* declaration,
                           clang::SourceManager& sourceManager);

private:
    boost::regex m_functionOrMethodNamePattern;
    std::unordered_set<std::string> m_reportedFunctionNames;
};
