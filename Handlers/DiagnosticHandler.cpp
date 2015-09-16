#include "Handlers/DiagnosticHandler.h"

#include "Common/Context.h"
#include "Common/FilenameHelper.h"
#include "Common/OutputPrinter.h"

#include <clang/Basic/SourceManager.h>

#include <boost/format.hpp>

using namespace clang;

DiagnosticHandler::DiagnosticHandler(Context& context)
    : m_context(context)
{}

void DiagnosticHandler::HandleDiagnostic(DiagnosticsEngine::Level level, const Diagnostic& info)
{
    if (level == DiagnosticsEngine::Level::Error || level == DiagnosticsEngine::Level::Fatal)
    {
        if (m_context.areWeInFakeHeaderSourceFile)
        {
            ReportDiagnostic("header file not self-contained",
                             Severity::Error,
                             "Including single header file should not result in compile error: %s",
                             info);
        }
        else
        {
            ReportDiagnostic("compile error",
                             Severity::Error,
                             "%s",
                             info);
        }
    }
    else if (level == DiagnosticsEngine::Level::Warning)
    {
        ReportDiagnostic("compile warning",
                         Severity::Warning,
                         "%s",
                         info);
    }
}

void DiagnosticHandler::ReportDiagnostic(const char* ruleName,
                                         Severity severity,
                                         const char* descriptionTemplate,
                                         const Diagnostic& info)
{
    std::string diagnosticString = GetDiagnosticString(info);

    SourceManager& sourceManager = info.getSourceManager();
    SourceLocation location = info.getLocation();

    std::string fileName = GetCleanFilename(location, sourceManager);
    int lineNumber = sourceManager.getPresumedLineNumber(location);

    std::string uniqueDiagnosticString = boost::str(boost::format("%s:%d: %s")
        % fileName % lineNumber % diagnosticString);

    if (m_reportedDiagnostics.count(uniqueDiagnosticString) == 0)
    {
        m_context.outputPrinter->PrintRuleViolation(
            ruleName,
            severity,
            boost::str(boost::format(descriptionTemplate) % diagnosticString),
            location,
            sourceManager);

        m_reportedDiagnostics.insert(uniqueDiagnosticString);
    }
}

std::string DiagnosticHandler::GetDiagnosticString(const Diagnostic& info)
{
    SmallVector<char, 100> outStr;
    info.FormatDiagnostic(outStr);
    outStr.push_back('\0');
    return std::string(outStr.data());
}
