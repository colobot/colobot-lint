#pragma once

#include "llvm/ADT/Hashing.h"
#include "llvm/ADT/StringRef.h"

#include <functional>

namespace std
{

template<>
struct hash<llvm::StringRef>
{
    size_t operator()(const llvm::StringRef& str) const
    {
        return llvm::hash_value(str);
    }
};

} // namespace std
