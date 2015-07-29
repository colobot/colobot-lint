#pragma once

#include "ASTCallbackRule.h"

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
    std::unordered_set<std::string> GetCandidateFieldsList(const clang::RecordDecl* recordDeclaration,
                                                           clang::ASTContext* context);

    enum class ConstructorStatus
    {
        NoConstructors,
        SomeConstructorsNotDefined,
        DefinedConstructors
    };

    ConstructorStatus CheckConstructorStatus(const clang::RecordDecl* recordDeclaration);
    void HandleConstructors(const clang::RecordDecl* recordDeclaration,
                            const std::unordered_set<std::string>& candidateFieldList,
                            clang::SourceManager& sourceManager);

    void HandleConstructorInitializationList(const clang::CXXConstructorDecl* constructorDeclaration,
                                             std::unordered_set<std::string>& candidateFieldList);

    void HandleConstructorBody(const clang::CXXConstructorDecl* constructorDeclaration,
                               std::unordered_set<std::string>& candidateFieldList);

    void HandleAssignStatement(const clang::BinaryOperator* assignStatement,
                               std::unordered_set<std::string>& candidateFieldList);

private:
    clang::ast_matchers::DeclarationMatcher m_matcher;
};
