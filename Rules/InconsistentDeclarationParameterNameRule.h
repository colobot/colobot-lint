#pragma once

#include "Rules/Rule.h"

#include <clang/ASTMatchers/ASTMatchFinder.h>

#include <unordered_set>

class InconsistentDeclarationParameterNameRule : public Rule,
                                                 public clang::ast_matchers::MatchFinder::MatchCallback
{
public:
    InconsistentDeclarationParameterNameRule(Context& context);

    void RegisterASTMatcherCallback(clang::ast_matchers::MatchFinder& finder) override;

    void run(const clang::ast_matchers::MatchFinder::MatchResult& result) override;

    static const char* GetName() { return "InconsistentDeclarationParameterNameRule"; }

private:
    bool HasInconsitentDeclarationParameters(const clang::FunctionDecl* functionDeclaration);

private:
    std::unordered_set<std::string> m_reportedFunctions;
};
