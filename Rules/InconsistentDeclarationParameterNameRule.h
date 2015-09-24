#pragma once

#include "Rules/Rule.h"

#include <clang/ASTMatchers/ASTMatchFinder.h>

#include <llvm/ADT/DenseSet.h>

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
    llvm::DenseSet<const clang::FunctionDecl*> m_visitedDeclarations;
};
