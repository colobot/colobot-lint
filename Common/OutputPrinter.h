#pragma once

#include "Severity.h"

#include "clang/Basic/SourceLocation.h"
#include "clang/AST/ASTContext.h"

#include <tinyxml.h>

class OutputPrinter
{
public:
    OutputPrinter(const std::string& outputFileName);

    void PrintRuleViolation(const std::string& ruleName,
                            Severity severity,
                            const std::string& description,
                            const clang::SourceLocation& location,
                            clang::SourceManager& sourceManager);

    void Save();

private:
    std::string GetSeverityString(Severity severity);

    std::string m_outputFileName;
    TiXmlDocument m_document;
    TiXmlElement* m_errorsElement;
};
