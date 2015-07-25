#pragma once

#include "ASTCallbackRule.h"

#include <clang/ASTMatchers/ASTMatchFinder.h>

#include <unordered_set>
#include <boost/regex.hpp>

class FunctionNamingRule : public ASTCallbackRule,
                           public clang::ast_matchers::MatchFinder::MatchCallback
{
public:
    FunctionNamingRule(Context& context);

    void RegisterASTMatcherCallback(clang::ast_matchers::MatchFinder& finder) override;

    void run(const clang::ast_matchers::MatchFinder::MatchResult& result) override;

    static const char* GetName() { return "FunctionNamingRule"; }

private:
    void HandleFunctionDeclaration(const clang::FunctionDecl* functionDeclaration,
                                   const clang::SourceLocation& location,
                                   clang::ASTContext* context);
    void HandleMethodDeclaration(const clang::CXXMethodDecl* methodDeclaration, clang::ASTContext* context);
    void ValidateName(const char* type, const std::string& name, const std::string& fulllyQualifiedName,
                      const clang::SourceLocation& location, clang::ASTContext* context);

private:
    clang::ast_matchers::DeclarationMatcher m_matcher;
    boost::regex m_functionOrMethodNamePattern;
    std::unordered_set<std::string> m_reportedFunctionNames;
};
