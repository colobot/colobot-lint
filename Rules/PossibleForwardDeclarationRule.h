#pragma once

#include "Rules/Rule.h"

#include <clang/ASTMatchers/ASTMatchFinder.h>

#include <llvm/ADT/DenseMap.h>
#include <llvm/ADT/DenseSet.h>

class PossibleForwardDeclarationRule : public Rule,
                                       public clang::ast_matchers::MatchFinder::MatchCallback
{
public:
    PossibleForwardDeclarationRule(Context& context);

    void RegisterASTMatcherCallback(clang::ast_matchers::MatchFinder& finder) override;

    void run(const clang::ast_matchers::MatchFinder::MatchResult& result) override;

    void onEndOfTranslationUnit();

    static const char* GetName() { return "PossibleForwardDeclarationRule"; }

private:
    bool IsInBlacklistedProjectHeader(const clang::Decl* declaration);
    void BlacklistIncludedProjectHeader(const clang::Decl* declaration);
    bool IsInDirectlyIncludedProjectHeader(const clang::Decl* declaration);
    bool IsForwardDeclarationPossible(const clang::TagDecl* tagDeclaration);

    void HandleDeclarationReferenceExpression(const clang::DeclRefExpr* declarationReferenceExpression);
    void HandleRecordDeclaration(const clang::CXXRecordDecl* recordDeclaration);
    void HandleDeclarationWithTagType(const clang::TagDecl* tagDeclaration,
                                      const clang::Decl* declarationWithTagType,
                                      bool isPointerOrReferenceType);
    void HandleExpressionWithTagType(const clang::TagDecl* tagDeclaration,
                                     const clang::Expr* expressionWithTagType);

private:
    clang::SourceManager* m_sourceManager;
    llvm::DenseMap<const clang::TagDecl*, clang::SourceLocation> m_candidateForwardDeclarations;
    llvm::DenseSet<clang::FileID> m_blacklistedProjectHeaders;
};
