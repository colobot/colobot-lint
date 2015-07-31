#pragma once

#include <llvm/ADT/StringRef.h>

#include <string>
#include <vector>

inline std::vector<llvm::StringRef> SplitLines(llvm::StringRef text)
{
    std::vector<llvm::StringRef> lines;
    while (! text.empty())
    {
        auto splitText = text.split('\n');
        lines.push_back(splitText.first.str());
        text = splitText.second;
    }
    return lines;
}
