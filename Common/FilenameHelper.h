#pragma once

#include <clang/Basic/SourceLocation.h>
#include <llvm/ADT/StringRef.h>

#include <string>

std::string CleanFilename(llvm::StringRef filename);

std::string GetCleanFilename(clang::SourceLocation location, clang::SourceManager& sourceManager);
