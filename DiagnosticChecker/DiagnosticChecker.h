#pragma once

#include "../Utils/OutputPrinter.h"

#include "clang/Tooling/Tooling.h"

class DiagnosticChecker : public clang::DiagnosticConsumer
{
public:
    DiagnosticChecker(OutputPrinter& outputPrinter);

    void HandleDiagnostic(clang::DiagnosticsEngine::Level level, const clang::Diagnostic& info) override;

private:
    std::string GetDiagnosticString(const clang::Diagnostic& info);

    OutputPrinter& m_outputPrinter;
};
