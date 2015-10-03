#include "Common/SourceLocationHelper.h"

#include "Common/Context.h"

#include <clang/Basic/SourceManager.h>

#include <llvm/ADT/STLExtras.h>
#include <llvm/Support/Path.h>

#include <vector>

using namespace llvm;
using namespace clang;

void SourceLocationHelper::SetContext(Context* context)
{
    m_context = context;
}

bool SourceLocationHelper::IsLocationOfInterest(StringRef ruleName,
                                                SourceLocation location,
                                                SourceManager& sourceManager)
{
    return IsLocationInMainFile(location, sourceManager) and
           not IsLocationInMacroExpansion(location, sourceManager) and
           not IsLocationInExclusionZone(ruleName, location, sourceManager);
}

bool SourceLocationHelper::IsLocationOfInterestAllowingMacros(StringRef ruleName,
                                                              SourceLocation location,
                                                              SourceManager& sourceManager)
{
    return IsLocationInMainFile(location, sourceManager) and
           not IsLocationInExclusionZone(ruleName, location, sourceManager);
}

bool SourceLocationHelper::IsLocationOfInterestIgnoringExclusionZone(SourceLocation location,
                                                                     SourceManager& sourceManager)
{
    return IsLocationInMainFile(location, sourceManager) and
           not IsLocationInMacroExpansion(location, sourceManager);
}

bool SourceLocationHelper::IsLocationInMainFile(SourceLocation location,
                                                SourceManager& sourceManager)
{
    if (m_context->areWeInFakeHeaderSourceFile)
    {
        StringRef fileName = GetCleanFilename(location, sourceManager);

        if (fileName.empty() || ! fileName.endswith(m_context->actualHeaderFileSuffix))
            return false;
    }
    else
    {
        if (! sourceManager.isInMainFile(location))
            return false;
    }

    return true;
}

bool SourceLocationHelper::IsLocationInMacroExpansion(SourceLocation location,
                                                      SourceManager& sourceManager)
{
    return sourceManager.isMacroArgExpansion(location) ||
           sourceManager.isMacroBodyExpansion(location);
}

bool SourceLocationHelper::IsLocationInExclusionZone(StringRef ruleName,
                                                     SourceLocation location,
                                                     SourceManager& sourceManager)
{
    if (m_context->exclusionZones.empty())
        return false;


    int lineNumber = sourceManager.getPresumedLineNumber(location);

    return m_context->exclusionZones.count(ExclusionZone{lineNumber, ruleName}) > 0 ||
           m_context->exclusionZones.count(ExclusionZone{lineNumber, StringRef("*")}) > 0;
}

bool SourceLocationHelper::IsLocationInProjectSourceFile(SourceLocation location, SourceManager& sourceManager)
{
    StringRef fileName = GetCleanFilename(location, sourceManager);
    for (const auto& path : m_context->projectLocalIncludePaths)
    {
        if (fileName.startswith(path))
            return true;
    }

    return false;
}

FileID SourceLocationHelper::GetMainFileID(SourceManager& sourceManager)
{
    if (! m_mainFileID.isInvalid())
        return m_mainFileID;

    if (m_context->areWeInFakeHeaderSourceFile)
    {
        for (auto it = sourceManager.fileinfo_begin();
             it != sourceManager.fileinfo_end();
             ++it)
        {
            if (StringRef(it->first->getName()).endswith(m_context->actualHeaderFileSuffix))
            {
                m_mainFileID = sourceManager.translateFile(it->first);
                break;
            }
        }
    }
    else
    {
        m_mainFileID = sourceManager.getMainFileID();
    }

    return m_mainFileID;
}

void SourceLocationHelper::ClearCachedData()
{
    m_mainFileID = FileID();
    m_cleanFilenameCache.clear();
}

StringRef SourceLocationHelper::GetCleanFilename(SourceLocation location, SourceManager& sourceManager)
{
    return GetCleanFilename(sourceManager.getFileID(location), sourceManager);
}

StringRef SourceLocationHelper::GetCleanFilename(FileID fileID, SourceManager& sourceManager)
{
    if (fileID.isInvalid())
        return StringRef("");

    auto it = m_cleanFilenameCache.find(fileID);
    if (it != m_cleanFilenameCache.end())
        return StringRef(it->second);

    const FileEntry* entry = sourceManager.getFileEntryForID(fileID);
    if (entry == nullptr)
        return StringRef("");

    m_cleanFilenameCache[fileID] = CleanRawFilename(entry->getName());
    it = m_cleanFilenameCache.find(fileID);
    return StringRef(it->second);
}

std::string SourceLocationHelper::CleanRawFilename(StringRef filename)
{
    std::vector<StringRef> pathComponents;

    for (auto it = sys::path::begin(filename);
         it != sys::path::end(filename);
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
