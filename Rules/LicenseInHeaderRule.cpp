#include "Rules/LicenseInHeaderRule.h"

#include "Common/Context.h"
#include "Common/FilenameHelper.h"
#include "Common/OutputPrinter.h"
#include "Common/SourceLocationHelper.h"

#include <clang/AST/ASTContext.h>

#include <boost/format.hpp>

using namespace clang;
using namespace llvm;

LicenseInHeaderRule::LicenseInHeaderRule(Context& context)
    : DirectASTConsumerRule(context)
{
}

void LicenseInHeaderRule::HandleTranslationUnit(clang::ASTContext& context)
{
    if (m_context.licenseTemplateLines.empty())
        return;

    SourceManager& sourceManager = context.getSourceManager();

    FileID mainFileID = m_context.sourceLocationHelper.GetMainFileID(sourceManager);

    std::string fileName = CleanFilename(StringRef(sourceManager.getFileEntryForID(mainFileID)->getName()));

    MemoryBuffer* buffer = sourceManager.getBuffer(mainFileID);
    const char* bufferChars = buffer->getBufferStart();
    int bufferSize = buffer->getBufferSize();
    if (bufferSize == 0)
        return;

    int bufferPos = 0;
    int lineNumber = 0;
    for (const auto& licenseLine : m_context.licenseTemplateLines)
    {
        ++lineNumber;
        StringRef line = GetNextBufferLine(bufferChars, bufferPos, bufferSize);
        bufferPos += line.size();
        bufferPos += 1; // newline character
        if (line != licenseLine)
        {
            m_context.printer.PrintRuleViolation(
                "license header",
                Severity::Style,
                boost::str(boost::format("File doesn't have proper license header; expected line was '%s'")
                    % licenseLine),
                fileName,
                lineNumber);

            break;
        }
    }
}

StringRef LicenseInHeaderRule::GetNextBufferLine(const char* bufferChars, int currentPos, int bufferSize)
{
    if (currentPos >= bufferSize)
        return StringRef();

    int endLinePos = currentPos;
    for (; endLinePos < bufferSize; ++endLinePos)
    {
        if (bufferChars[endLinePos] == '\n')
            break;
    }

    if (endLinePos == currentPos)
        return StringRef();

    return StringRef(&bufferChars[currentPos], endLinePos - currentPos);
}
