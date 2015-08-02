#include "Rules/WhitespaceRule.h"

#include "Common/Context.h"
#include "Common/FilenameHelper.h"
#include "Common/OutputPrinter.h"

#include <clang/AST/ASTContext.h>

using namespace clang;
using namespace llvm;

WhitespaceRule::WhitespaceRule(Context& context)
    : DirectASTConsumerRule(context)
{}

void WhitespaceRule::HandleTranslationUnit(ASTContext& context)
{
    SourceManager& sourceManager = context.getSourceManager();

    FileID mainFileID = GetMainFileID(sourceManager);

    std::string fileName = CleanFilename(StringRef(sourceManager.getFileEntryForID(mainFileID)->getName()));

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
            m_context.printer.PrintRuleViolation(
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
                m_context.printer.PrintRuleViolation(
                    "whitespace",
                    Severity::Style,
                    "Whitespace at end of line",
                    fileName,
                    lineNumber);
            }
            if (numberOfTabs > 0)
            {
                m_context.printer.PrintRuleViolation(
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
        m_context.printer.PrintRuleViolation(
            "whitespace",
            Severity::Style,
            "File should end with newline",
            fileName,
            lineNumber-1);
    }
}

FileID WhitespaceRule::GetMainFileID(SourceManager& sourceManager)
{
    FileID mainFileID{};

    if (m_context.areWeInFakeHeaderSourceFile)
    {
        for (auto it = sourceManager.fileinfo_begin();
             it != sourceManager.fileinfo_end();
             ++it)
        {
            if (StringRef(it->first->getName()).endswith(m_context.actualHeaderFileSuffix))
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

