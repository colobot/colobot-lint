#pragma once

#include "Rules/Rule.h"

#include <clang/ASTMatchers/ASTMatchFinder.h>

class LicenseInHeaderRule : public Rule,
                            public clang::ast_matchers::MatchFinder::MatchCallback
{
public:
    LicenseInHeaderRule(Context& context);

    void RegisterASTMatcherCallback(clang::ast_matchers::MatchFinder& finder);

    void run(const clang::ast_matchers::MatchFinder::MatchResult& Result) override;

    static const char* GetName() { return "LicenseInHeaderRule"; }

private:
    llvm::StringRef GetNextBufferLine(const char* bufferChars, int currentPos, int bufferSize);
};
