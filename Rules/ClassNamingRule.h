#pragma once

#include "Rules/Rule.h"

#include <clang/ASTMatchers/ASTMatchFinder.h>

#include <llvm/ADT/DenseSet.h>

#include <boost/regex.hpp>

class ClassNamingRule : public Rule,
                        public clang::ast_matchers::MatchFinder::MatchCallback
{
public:
    ClassNamingRule(Context& context);

    void RegisterASTMatcherCallback(clang::ast_matchers::MatchFinder& finder) override;

    void run(const clang::ast_matchers::MatchFinder::MatchResult& result) override;

    static const char* GetName() { return "ClassNamingRule"; }

private:
    boost::regex m_classNamePattern;
    boost::regex m_structOrUnionNamePattern;
    llvm::DenseSet<const clang::RecordDecl*> m_visitedDeclarations;
};
