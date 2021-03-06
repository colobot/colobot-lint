#pragma once

#include "Rules/Rule.h"

#include <clang/ASTMatchers/ASTMatchFinder.h>

#include <boost/regex.hpp>

class VariableNamingRule : public Rule,
                           public clang::ast_matchers::MatchFinder::MatchCallback
{
public:
    VariableNamingRule(Context& context);

    void RegisterASTMatcherCallback(clang::ast_matchers::MatchFinder& finder) override;

    void run(const clang::ast_matchers::MatchFinder::MatchResult& result) override;

    static const char* GetName() { return "VariableNamingRule"; }

private:
    void HandleVariableDeclaration(const clang::VarDecl* variableDeclaration,
                                   clang::SourceManager& sourceManager);

    void HandleFieldDeclaration(const clang::FieldDecl* fieldDeclaration,
                                clang::SourceManager& sourceManager);

    void ValidateFieldDeclaration(clang::StringRef name,
                                  clang::AccessSpecifier access,
                                  clang::SourceLocation location,
                                  clang::SourceManager& sourceManager);

private:
    boost::regex m_localVariableNamePattern;
    boost::regex m_nonConstGlobalVariableNamePattern;
    boost::regex m_constGlobalVariableNamePattern;
    boost::regex m_deprecatedVariableNamePattern;
    boost::regex m_publicFieldNamePattern;
    boost::regex m_privateOrProtectedFieldNamePattern;
    boost::regex m_deprecatedFieldNamePattern;
};
