#pragma once

#include "Rules/ASTCallbackRule.h"

#include <clang/ASTMatchers/ASTMatchFinder.h>

#include <unordered_map>
#include <unordered_set>

class UnusedForwardDeclarationRule : public ASTCallbackRule,
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
    std::unordered_set<const clang::TagDecl*> m_definedDeclarations;
    std::unordered_map<const clang::TagDecl*, int> m_usesOfForwardDeclarations;
};
