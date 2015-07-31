#pragma once

#include "Rules/ASTCallbackRule.h"

#include <clang/ASTMatchers/ASTMatchFinder.h>

class UninitializedLocalVariableRule : public ASTCallbackRule,
                                       public clang::ast_matchers::MatchFinder::MatchCallback
{
public:
    UninitializedLocalVariableRule(Context& context);

    void RegisterASTMatcherCallback(clang::ast_matchers::MatchFinder& finder) override;

    void run(const clang::ast_matchers::MatchFinder::MatchResult& result) override;

    static const char* GetName() { return "UninitializedLocalVariableRule"; }

private:
    bool HasImplicitInitialization(const clang::VarDecl* variableDeclaration);

private:
    clang::ast_matchers::DeclarationMatcher m_matcher;
};
