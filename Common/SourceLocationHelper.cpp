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
