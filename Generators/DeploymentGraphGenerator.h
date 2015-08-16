#pragma once

#include "Generators/Generator.h"

#include <clang/ASTMatchers/ASTMatchFinder.h>

class DeploymentGraphGenerator : public Generator,
                                 public clang::ast_matchers::MatchFinder::MatchCallback
{
public:
    DeploymentGraphGenerator(Context& context);

    void RegisterASTMatcherCallback(clang::ast_matchers::MatchFinder& finder) override;

    void run(const clang::ast_matchers::MatchFinder::MatchResult& result) override;

    static const char* GetName() { return "DeploymentGraph"; }

private:
    void HandleRecordDeclaration(const clang::CXXRecordDecl* recordDecl,
                                 clang::SourceManager& sourceManager);
    void HandleUniquePtrFieldDeclaration(const clang::FieldDecl* fieldDecl,
                                         const clang::QualType* uniquePtrType,
                                         clang::ASTContext* astContext);

private:
    clang::ast_matchers::DeclarationMatcher m_recordDeclMatcher;
    clang::ast_matchers::DeclarationMatcher m_uniquePtrFieldDeclMatcher;
};
