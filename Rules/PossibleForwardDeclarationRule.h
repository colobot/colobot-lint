#pragma once

#include "Rules/ASTCallbackRule.h"

#include <clang/ASTMatchers/ASTMatchFinder.h>

#include <unordered_map>

class PossibleForwardDeclarationRule : public ASTCallbackRule,
                                       public clang::ast_matchers::MatchFinder::MatchCallback
{
public:
    PossibleForwardDeclarationRule(Context& context);

    void RegisterASTMatcherCallback(clang::ast_matchers::MatchFinder& finder) override;

    void run(const clang::ast_matchers::MatchFinder::MatchResult& result) override;

    void onEndOfTranslationUnit();

    static const char* GetName() { return "PossibleForwardDeclarationRule"; }

private:
    bool IsCandidateForForwardDeclaration(const clang::TagDecl* tagDeclaration);
    void HandleDeclarationWithTagType(const clang::TagDecl* tagDeclaration,
                                      const clang::Decl* declarationWithTagType,
                                      bool isPointerOrReferenceType);
    void HandleExpressionWithTagType(const clang::TagDecl* tagDeclaration,
                                     const clang::Expr* expressionWithTagType);

private:
    clang::SourceManager* m_sourceManager;
    std::unordered_map<const clang::TagDecl*, clang::SourceLocation> m_candidateForwardDeclarations;
};
