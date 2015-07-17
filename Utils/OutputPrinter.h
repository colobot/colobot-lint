#pragma once

#include "clang/Basic/SourceLocation.h"
#include "clang/AST/ASTContext.h"

#include <fstream>

class OutputPrinter
{
public:
    OutputPrinter(const std::string& outputFileName);

    void PrintRuleViolation(const std::string& ruleName,
                            const std::string& violationDescription,
                            const clang::SourceLocation& location,
                            clang::ASTContext* context);

private:
    std::ofstream m_outputFile;
};
