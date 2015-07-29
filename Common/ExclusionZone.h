#pragma once

#include "StringRefHash.h"

#include <llvm/ADT/StringRef.h>

struct ExclusionZone
{
    int lineNumber;
    llvm::StringRef ruleName;
};

namespace std
{
template<>
struct hash<ExclusionZone>
{
    size_t operator()(const ExclusionZone& exclusionZone) const
    {
        size_t h1 = hash<int>()(exclusionZone.lineNumber);
        size_t h2 = hash<llvm::StringRef>()(exclusionZone.ruleName);
        return h1 ^ (h2 << 1);
    }
};
} // namespace std

inline bool operator==(const ExclusionZone& left, const ExclusionZone& right)
{
    return left.lineNumber == right.lineNumber &&
           left.ruleName == right.ruleName;
}