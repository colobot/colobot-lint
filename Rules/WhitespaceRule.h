#pragma once

#include "Rules/DirectASTConsumerRule.h"

#include <clang/AST/RecursiveASTVisitor.h>

#include <unordered_set>

class WhitespaceRule : public DirectASTConsumerRule
{
public:
    WhitespaceRule(Context& context);

    void HandleTranslationUnit(clang::ASTContext &context) override;

    static const char* GetName() { return "WhitespaceRule"; }

private:
    clang::FileID GetMainFileID(clang::SourceManager& sourceManager);
};
