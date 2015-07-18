#include "DiagnosticChecker.h"

using namespace clang;

DiagnosticChecker::DiagnosticChecker(OutputPrinter& outputPrinter)
    : m_outputPrinter(outputPrinter)
{}

void DiagnosticChecker::HandleDiagnostic(DiagnosticsEngine::Level level, const Diagnostic& info)
{
    if (level == DiagnosticsEngine::Level::Error)
    {
        m_outputPrinter.PrintRuleViolation("compile error",
                                           Severity::Error,
                                           GetDiagnosticString(info),
                                           info.getLocation(),
                                           info.getSourceManager());
    }
    else if (level == DiagnosticsEngine::Level::Warning)
    {
        m_outputPrinter.PrintRuleViolation("compile warning",
                                           Severity::Warning,
                                           GetDiagnosticString(info),
                                           info.getLocation(),
                                           info.getSourceManager());
    }
}

std::string DiagnosticChecker::GetDiagnosticString(const Diagnostic& info)
{
    SmallVector<char, 100> outStr;
    info.FormatDiagnostic(outStr);
    outStr.push_back('\0');
    return std::string(outStr.data());
}
