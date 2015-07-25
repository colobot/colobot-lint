#pragma once

#include "ASTCallbackRule.h"

#include <clang/ASTMatchers/ASTMatchFinder.h>

#include <boost/regex.hpp>

class EnumNamingRule : public ASTCallbackRule,
                       public clang::ast_matchers::MatchFinder::MatchCallback
{
public:
    EnumNamingRule(Context& context);

    void RegisterASTMatcherCallback(clang::ast_matchers::MatchFinder& finder) override;

    void run(const clang::ast_matchers::MatchFinder::MatchResult& result) override;

    static const char* GetName() { return "EnumNamingRule"; }

private:
    void HandleEnumDeclaration(const clang::EnumDecl* enumDeclaration, clang::ASTContext* context);
    void HandleEnumConstantDeclaration(const clang::EnumConstantDecl* enumConstantDeclaration, clang::ASTContext* context);

private:
    clang::ast_matchers::DeclarationMatcher m_enumDeclarationMatcher;
    clang::ast_matchers::DeclarationMatcher m_enumConstantDeclarationMatcher;
    boost::regex m_enumNamePattern;
    boost::regex m_enumConstantPattern;
};
