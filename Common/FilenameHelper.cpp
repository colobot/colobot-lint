#include "Common/FilenameHelper.h"

#include <clang/Basic/SourceManager.h>
#include <llvm/Support/Path.h>

#include <vector>

using namespace llvm;
using namespace clang;

std::string CleanFilename(StringRef filename)
{
    std::vector<StringRef> pathComponents;

    for (auto it = llvm::sys::path::begin(filename);
         it != llvm::sys::path::end(filename);
         ++it)
    {
        StringRef component = *it;
        if (component == "." || component == "/")
        {}
        else if (component == "..")
        {
            if (!pathComponents.empty())
                pathComponents.pop_back();
        }
        else
        {
            pathComponents.push_back(component);
        }
    }

    std::string cleanFilename;
    for (const auto& component : pathComponents)
    {
        cleanFilename += "/";
        cleanFilename += component.str();
    }

    return cleanFilename;
}

std::string GetCleanFilename(SourceLocation location, SourceManager& sourceManager)
{
    return CleanFilename(sourceManager.getFilename(location));
}
