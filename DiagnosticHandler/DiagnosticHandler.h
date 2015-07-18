#pragma once

#include "../Common/Context.h"

#include "clang/Tooling/Tooling.h"

class DiagnosticHandler : public clang::DiagnosticConsumer
{
public:
    DiagnosticHandler(Context& context);

    void HandleDiagnostic(clang::DiagnosticsEngine::Level level, const clang::Diagnostic& info) override;

private:
    std::string GetDiagnosticString(const clang::Diagnostic& info);

    Context& m_context;
};
