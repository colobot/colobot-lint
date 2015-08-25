#pragma once

#include "Rules/ASTCallbackRule.h"

#include <clang/ASTMatchers/ASTMatchFinder.h>

#include <unordered_set>

struct IncludeDirective
{
    clang::SourceLocation location = {};
    std::string includeFileName = {};
    std::string fullFileName = {};
    bool isAngled = false;
};

using IncludeDirectives = std::vector<IncludeDirective>;
using IncludeDirectiveIt = IncludeDirectives::const_iterator;

class IncludeStyleRule : public ASTCallbackRule,
                         public clang::ast_matchers::MatchFinder::MatchCallback
{
public:
    IncludeStyleRule(Context& context);

    void RegisterPreProcessorCallbacks(clang::CompilerInstance& compiler) override;
    void RegisterASTMatcherCallback(clang::ast_matchers::MatchFinder& finder) override;

    void run(const clang::ast_matchers::MatchFinder::MatchResult& result) override;

    void AtEndOfMainFile(const IncludeDirectives& includeDirectives,
                         clang::SourceManager& sourceManager);

    static const char* GetName() { return "IncludeStyleRule"; }

private:
    void CheckAngledBrackets(const IncludeDirectives& includeDirectives, clang::SourceManager& sourceManager);

    IncludeDirectiveIt CheckFirstInclude(IncludeDirectiveIt startIt, IncludeDirectiveIt endIt, clang::SourceManager& sourceManager);
    IncludeDirectiveIt CheckConfigInclude(IncludeDirectiveIt startIt, IncludeDirectiveIt endIt, clang::SourceManager& sourceManager);
    IncludeDirectiveIt CheckLocalIncludes(IncludeDirectiveIt startIt, IncludeDirectiveIt endIt, clang::SourceManager& sourceManager);
    void CheckGlobalIncludes(IncludeDirectiveIt startIt, IncludeDirectiveIt endIt, clang::SourceManager& sourceManager);

    void CheckNewBlock(IncludeDirectiveIt currentIt, IncludeDirectiveIt endIt, clang::SourceManager& sourceManager);
    void CheckIncludeRangeIsSorted(IncludeDirectiveIt startIt, IncludeDirectiveIt endIt, clang::SourceManager& sourceManager);

    bool IsLocalInclude(const std::string& fileName);
    std::string GetProjectIncludeSubpath(const std::string& fileName);
    std::string GetMatchingHeaderFileName(clang::SourceManager& sourceManager);

private:
    std::unordered_set<std::string> m_possibleMainClassBaseIncludes;
};
