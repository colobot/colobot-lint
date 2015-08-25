#pragma once

#include "Rules/ASTCallbackRule.h"

#include <clang/ASTMatchers/ASTMatchFinder.h>

#include <unordered_set>

class OldStyleFunctionRule : public ASTCallbackRule,
                             public clang::ast_matchers::MatchFinder::MatchCallback
{
public:
    OldStyleFunctionRule(Context& context);

    void RegisterASTMatcherCallback(clang::ast_matchers::MatchFinder& finder) override;

    void run(const clang::ast_matchers::MatchFinder::MatchResult& result) override;

    static const char* GetName() { return "OldStyleFunctionRule"; }

private:
    std::string GetShortDeclarationsString(const std::vector<llvm::StringRef>& declarations, int totalCount);
};
