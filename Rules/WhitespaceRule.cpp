#include "Rules/WhitespaceRule.h"

#include "Common/Context.h"
#include "Common/OutputPrinter.h"
#include "Common/SourceLocationHelper.h"
#include "Common/TranslationUnitMatcher.h"

#include <clang/AST/ASTContext.h>

using namespace clang;
using namespace clang::ast_matchers;
using namespace llvm;

WhitespaceRule::WhitespaceRule(Context& context)
    : Rule(context)
{}

void WhitespaceRule::RegisterASTMatcherCallback(ast_matchers::MatchFinder& finder)
{
    finder.addMatcher(customTranslationUnitDecl().bind("translationUnitDecl"), this);
}

void WhitespaceRule::run(const ast_matchers::MatchFinder::MatchResult& result)
{
    const TranslationUnitDecl* translationUnitDeclaration = result.Nodes.getNodeAs<TranslationUnitDecl>("translationUnitDecl");
    if (translationUnitDeclaration == nullptr)
        return;

    SourceManager& sourceManager = *result.SourceManager;

    FileID mainFileID = m_context.sourceLocationHelper.GetMainFileID(sourceManager);

    StringRef fileName = m_context.sourceLocationHelper.GetCleanFilename(mainFileID, sourceManager);

    MemoryBuffer* buffer = sourceManager.getBuffer(mainFileID);
    const char* bufferChars = buffer->getBufferStart();
    int bufferSize = buffer->getBufferSize();
    if (bufferSize == 0)
        return;

    int lineNumber = 1;
    bool haveWhitespace = false;
    int numberOfTabs = 0;
    for (int i = 0; i < bufferSize; ++i)
    {
        char ch = bufferChars[i];

        if (ch == '\r')
        {
            m_context.outputPrinter->PrintRuleViolation(
                "whitespace",
                Severity::Style,
                "File seems to have DOS style line endings",
                fileName,
                lineNumber);

            return;
        }

        if (ch == '\n')
        {
            if (haveWhitespace)
            {
                m_context.outputPrinter->PrintRuleViolation(
                    "whitespace",
                    Severity::Style,
                    "Whitespace at end of line",
                    fileName,
                    lineNumber);
            }
            if (numberOfTabs > 0)
            {
                m_context.outputPrinter->PrintRuleViolation(
                    "whitespace",
                    Severity::Style,
                    "Tab character is not allowed as whitespace",
                    fileName,
                    lineNumber);
            }

            ++lineNumber;
            haveWhitespace = false;
            numberOfTabs = 0;
            continue;
        }

        if (ch == '\t')
        {
            ++numberOfTabs;
            haveWhitespace = true;
            continue;
        }

        if (ch == ' ')
        {
            haveWhitespace = true;
            continue;
        }

        haveWhitespace = false;
    }

    if (bufferChars[bufferSize - 1] != '\n')
    {
        m_context.outputPrinter->PrintRuleViolation(
            "whitespace",
            Severity::Style,
            "File should end with newline",
            fileName,
            lineNumber-1);
    }
}
