#pragma once

#include "Rules/DirectASTConsumerRule.h"

class LicenseInHeaderRule : public DirectASTConsumerRule
{
public:
    LicenseInHeaderRule(Context& context);

    void HandleTranslationUnit(clang::ASTContext &context) override;

    static const char* GetName() { return "LicenseInHeaderRule"; }

private:
    llvm::StringRef GetNextBufferLine(const char* bufferChars, int currentPos, int bufferSize);
};
