#include "Rules/LicenseInHeaderRule.h"

#include "Common/Context.h"
#include "Common/OutputPrinter.h"
#include "Common/SourceLocationHelper.h"
#include "Common/TranslationUnitMatcher.h"

#include <clang/AST/ASTContext.h>

#include <boost/format.hpp>

using namespace clang;
using namespace clang::ast_matchers;
using namespace llvm;

LicenseInHeaderRule::LicenseInHeaderRule(Context& context)
    : Rule(context)
{
}

void LicenseInHeaderRule::RegisterASTMatcherCallback(ast_matchers::MatchFinder& finder)
{
    finder.addMatcher(customTranslationUnitDecl().bind("translationUnitDecl"), this);
}

void LicenseInHeaderRule::run(const ast_matchers::MatchFinder::MatchResult& result)
{
    const TranslationUnitDecl* translationUnitDeclaration = result.Nodes.getNodeAs<TranslationUnitDecl>("translationUnitDecl");
    if (translationUnitDeclaration == nullptr)
        return;

    if (m_context.licenseTemplateLines.empty())
        return;

    SourceManager& sourceManager = *result.SourceManager;

    FileID mainFileID = m_context.sourceLocationHelper.GetMainFileID(sourceManager);

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
            StringRef fileName = m_context.sourceLocationHelper.GetCleanFilename(mainFileID, sourceManager);

            m_context.outputPrinter->PrintRuleViolation(
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
