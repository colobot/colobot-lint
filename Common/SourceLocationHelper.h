#pragma once

#include <llvm/ADT/StringRef.h>

struct Context;

namespace clang
{
class SourceLocation;
class SourceManager;
}

class SourceLocationHelper
{
public:
    void SetContext(Context* context);

    bool IsLocationOfInterest(llvm::StringRef ruleName,
                              clang::SourceLocation location,
                              clang::SourceManager& sourceManager);

    bool IsLocationOfInterestIgnoringExclusionZone(clang::SourceLocation location,
                                                   clang::SourceManager& sourceManager);

    bool IsLocationInProjectSourceFile(clang::SourceLocation location,
                                       clang::SourceManager& sourceManager);

private:
    Context* m_context = nullptr;
};
