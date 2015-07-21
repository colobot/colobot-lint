#include "SourceLocationHelper.h"

#include "Context.h"

#include "clang/Basic/SourceManager.h"

void SourceLocationHelper::SetContext(Context* context)
{
    m_context = context;
}

bool SourceLocationHelper::IsLocationOfInterest(const clang::SourceLocation& location,
                                                clang::SourceManager& sourceManager)
{
    // completely ignore macros
    if (sourceManager.isMacroArgExpansion(location) ||
        sourceManager.isMacroBodyExpansion(location))
        return false;

    if (m_context->areWeInFakeHeaderSourceFile)
    {
        return sourceManager.getFilename(location).endswith(m_context->actualHeaderFileSuffix);
    }

    return sourceManager.isInMainFile(location);
}
