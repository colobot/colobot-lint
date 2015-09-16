#pragma once

#include "Common/Severity.h"

#include <clang/Basic/Diagnostic.h>

#include <unordered_set>

struct Context;

class DiagnosticHandler : public clang::DiagnosticConsumer
{
public:
    DiagnosticHandler(Context& context);

    void HandleDiagnostic(clang::DiagnosticsEngine::Level level, const clang::Diagnostic& info) override;

private:
    void ReportDiagnostic(const char* ruleName,
                          Severity severity,
                          const char* descriptionTemplate,
                          const clang::Diagnostic& info);
    std::string GetDiagnosticString(const clang::Diagnostic& info);

    Context& m_context;
    std::unordered_set<std::string> m_reportedDiagnostics;
};
