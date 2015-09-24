#pragma once

#include "Rules/Rule.h"

#include <clang/ASTMatchers/ASTMatchFinder.h>

#include <llvm/ADT/DenseMap.h>
#include <llvm/ADT/DenseSet.h>

class UnusedForwardDeclarationRule : public Rule,
                                     public clang::ast_matchers::MatchFinder::MatchCallback
{
public:
    UnusedForwardDeclarationRule(Context& context);

    void RegisterASTMatcherCallback(clang::ast_matchers::MatchFinder& finder) override;

    void run(const clang::ast_matchers::MatchFinder::MatchResult& result) override;

    void onEndOfTranslationUnit() override;

    static const char* GetName() { return "UnusedForwardDeclarationRule"; }

private:
    void HandleDefinition(const clang::TagDecl* tagDeclaration);
    void HandleForwardDeclaration(const clang::TagDecl* tagDeclaration);
    void HandleTypeReference(const clang::TagDecl* tagDeclaration);

private:
    clang::SourceManager* m_sourceManager = nullptr;
    llvm::DenseSet<const clang::TagDecl*> m_definedDeclarations;
    llvm::DenseMap<const clang::TagDecl*, int> m_usesOfForwardDeclarations;
};
