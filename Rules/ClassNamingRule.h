#pragma once

#include "Rules/ASTCallbackRule.h"

#include <clang/ASTMatchers/ASTMatchFinder.h>

#include <unordered_set>
#include <boost/regex.hpp>

class ClassNamingRule : public ASTCallbackRule,
                        public clang::ast_matchers::MatchFinder::MatchCallback
{
public:
    ClassNamingRule(Context& context);

    void RegisterASTMatcherCallback(clang::ast_matchers::MatchFinder& finder) override;

    void run(const clang::ast_matchers::MatchFinder::MatchResult& result) override;

    static const char* GetName() { return "ClassNamingRule"; }

private:
    std::string GetRecordTypeString(const clang::RecordDecl* recordDeclaration);
    std::string GetLowercaseRecordTypeString(const clang::RecordDecl* recordDeclaration);

private:
    boost::regex m_classNamePattern;
    boost::regex m_structOrUnionNamePattern;
    std::unordered_set<std::string> m_reportedNames;
};
