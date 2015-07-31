#pragma once

#include "Common/Severity.h"

#include <tinyxml.h>

namespace clang
{
class SourceLocation;
class SourceManager;
} // namespace clang

class OutputPrinter
{
public:
    OutputPrinter(const std::string& outputFileName);

    void PrintRuleViolation(const std::string& ruleName,
                            Severity severity,
                            const std::string& description,
                            clang::SourceLocation location,
                            clang::SourceManager& sourceManager,
                            int lineOffset = 0);

    void PrintRuleViolation(const std::string& ruleName,
                            Severity severity,
                            const std::string& description,
                            const std::string& fileName,
                            int lineNumber);

    void Save();

private:
    std::string GetSeverityString(Severity severity);

    std::string m_outputFileName;
    TiXmlDocument m_document;
    TiXmlElement* m_errorsElement;
};
