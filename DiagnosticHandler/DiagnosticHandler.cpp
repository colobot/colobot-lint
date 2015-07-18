#include "DiagnosticHandler.h"

using namespace clang;

DiagnosticHandler::DiagnosticHandler(Context& context)
    : m_context(context)
{}

void DiagnosticHandler::HandleDiagnostic(DiagnosticsEngine::Level level, const Diagnostic& info)
{
    if (level == DiagnosticsEngine::Level::Error)
    {
        m_context.printer.PrintRuleViolation("compile error",
                                             Severity::Error,
                                             GetDiagnosticString(info),
                                             info.getLocation(),
                                             info.getSourceManager());
    }
    else if (level == DiagnosticsEngine::Level::Warning)
    {
        m_context.printer.PrintRuleViolation("compile warning",
                                             Severity::Warning,
                                             GetDiagnosticString(info),
                                             info.getLocation(),
                                             info.getSourceManager());
    }
}

std::string DiagnosticHandler::GetDiagnosticString(const Diagnostic& info)
{
    SmallVector<char, 100> outStr;
    info.FormatDiagnostic(outStr);
    outStr.push_back('\0');
    return std::string(outStr.data());
}
