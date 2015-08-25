#pragma once

#include "Rules/ASTCallbackRule.h"

#include "Common/StringRefHash.h"

#include <clang/ASTMatchers/ASTMatchFinder.h>

#include <unordered_set>

class UninitializedFieldRule : public ASTCallbackRule,
                                public clang::ast_matchers::MatchFinder::MatchCallback
{
public:
    UninitializedFieldRule(Context& context);

    void RegisterASTMatcherCallback(clang::ast_matchers::MatchFinder& finder) override;

    void run(const clang::ast_matchers::MatchFinder::MatchResult& result) override;

    static const char* GetName() { return "UninitializedFieldRule"; }

private:
    void HandleRecordDeclaration(const clang::RecordDecl* recordDeclaration,
                                 clang::ASTContext* context);

    void HandleConstructorDeclaration(const clang::CXXConstructorDecl* constructorDeclaration,
                                      clang::ASTContext* context);

    bool AreThereInterestingConstructorDeclarations(const clang::RecordDecl* recordDeclaration);

    using StringRefSet = std::unordered_set<llvm::StringRef>;

    StringRefSet GetCandidateFieldsList(const clang::RecordDecl* recordDeclaration,
                                        clang::ASTContext* context);

    void CheckInitializationsInInitializationList(const clang::CXXConstructorDecl* constructorDeclaration,
                                                  StringRefSet& candidateFieldList);

    void CheckInitializationsInConstructorBody(const clang::CXXConstructorDecl* constructorDeclaration,
                                               StringRefSet& candidateFieldList);

    void CheckInitializationsInAssignStatement(const clang::BinaryOperator* assignStatement,
                                               StringRefSet& candidateFieldList);

private:
    std::unordered_set<const clang::CXXConstructorDecl*> m_alreadyHandledConstructorDeclarations;
};
