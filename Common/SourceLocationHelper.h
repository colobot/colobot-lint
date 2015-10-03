#pragma once

#include <clang/Basic/SourceLocation.h>

#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/DenseMap.h>

#include <string>

struct Context;

class SourceLocationHelper
{
public:
    void SetContext(Context* context);

    bool IsLocationOfInterest(llvm::StringRef ruleName,
                              clang::SourceLocation location,
                              clang::SourceManager& sourceManager);

    bool IsLocationOfInterestAllowingMacros(llvm::StringRef ruleName,
                                            clang::SourceLocation location,
                                            clang::SourceManager& sourceManager);

    bool IsLocationOfInterestIgnoringExclusionZone(clang::SourceLocation location,
                                                   clang::SourceManager& sourceManager);

    bool IsLocationInProjectSourceFile(clang::SourceLocation location,
                                       clang::SourceManager& sourceManager);

    clang::FileID GetMainFileID(clang::SourceManager& sourceManager);

    void ClearCachedData();

    clang::StringRef GetCleanFilename(clang::SourceLocation location, clang::SourceManager& sourceManager);
    clang::StringRef GetCleanFilename(clang::FileID fileID, clang::SourceManager& sourceManager);
    std::string CleanRawFilename(llvm::StringRef filename);

private:
    bool IsLocationInMainFile(clang::SourceLocation location,
                              clang::SourceManager& sourceManager);
    bool IsLocationInMacroExpansion(clang::SourceLocation location,
                                    clang::SourceManager& sourceManager);
    bool IsLocationInExclusionZone(llvm::StringRef ruleName,
                                   clang::SourceLocation location,
                                   clang::SourceManager& sourceManager);

    Context* m_context = nullptr;
    clang::FileID m_mainFileID;
    llvm::DenseMap<clang::FileID, std::string> m_cleanFilenameCache;
};
