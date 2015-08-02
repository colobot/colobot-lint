#pragma once

#include <clang/Basic/SourceLocation.h>

#include <string>

std::string GetCleanFilename(clang::SourceLocation location, clang::SourceManager& sourceManager);
