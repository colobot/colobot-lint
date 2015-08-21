#include "Common/SourceLocationHelper.h"

#include "Common/Context.h"
#include "Common/FilenameHelper.h"

#include <clang/Basic/SourceManager.h>

using namespace llvm;
using namespace clang;

void SourceLocationHelper::SetContext(Context* context)
{
    m_context = context;
}

bool SourceLocationHelper::IsLocationOfInterestIgnoringExclusionZone(SourceLocation location,
                                                                     SourceManager& sourceManager)
{
    if (m_context->areWeInFakeHeaderSourceFile)
    {
        std::string fileName = GetCleanFilename(location, sourceManager);

        if (! StringRef(fileName).endswith(m_context->actualHeaderFileSuffix))
            return false;
    }
    else
    {
        if (! sourceManager.isInMainFile(location))
            return false;
    }

    // completely ignore macros
    if (sourceManager.isMacroArgExpansion(location) ||
        sourceManager.isMacroBodyExpansion(location))
    {
        return false;
    }

    return true;
}

bool SourceLocationHelper::IsLocationOfInterest(StringRef ruleName,
                                                SourceLocation location,
                                                SourceManager& sourceManager)
{
    if (! IsLocationOfInterestIgnoringExclusionZone(location, sourceManager))
        return false;

    if (! m_context->exclusionZones.empty())
    {
        int lineNumber = sourceManager.getPresumedLineNumber(location);

        if (m_context->exclusionZones.count(ExclusionZone{lineNumber, ruleName}) > 0 ||
            m_context->exclusionZones.count(ExclusionZone{lineNumber, StringRef("*")}) > 0)
        {
            return false;
        }
    }

    return true;
}

bool SourceLocationHelper::IsLocationInProjectSourceFile(SourceLocation location, SourceManager& sourceManager)
{
    std::string fileName = GetCleanFilename(location, sourceManager);
    for (const auto& path : m_context->projectLocalIncludePaths)
    {
        if (StringRef(fileName).startswith(path))
            return true;
    }

    return false;
}

FileID SourceLocationHelper::GetMainFileID(SourceManager& sourceManager)
{
    FileID mainFileID{};

    if (m_context->areWeInFakeHeaderSourceFile)
    {
        for (auto it = sourceManager.fileinfo_begin();
             it != sourceManager.fileinfo_end();
             ++it)
        {
            if (StringRef(it->first->getName()).endswith(m_context->actualHeaderFileSuffix))
            {
                mainFileID = sourceManager.translateFile(it->first);
                break;
            }
        }
    }
    else
    {
        mainFileID = sourceManager.getMainFileID();
    }

    return mainFileID;
}
