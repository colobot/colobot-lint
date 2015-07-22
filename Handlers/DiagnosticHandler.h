#pragma once

#include <clang/Basic/Diagnostic.h>

struct Context;

class DiagnosticHandler : public clang::DiagnosticConsumer
{
public:
    DiagnosticHandler(Context& context);

    void HandleDiagnostic(clang::DiagnosticsEngine::Level level, const clang::Diagnostic& info) override;

private:
    std::string GetDiagnosticString(const clang::Diagnostic& info);

    Context& m_context;
};
