#pragma once

#include "Common/Severity.h"

#include <memory>
#include <string>

namespace clang
{
class SourceLocation;
class SourceManager;
} // namespace clang


enum class OutputFormat
{
    PlainTextReport,
    XmlReport,
    DotGraph
};

class OutputPrinter
{
protected:
    OutputPrinter(const std::string& outputFileName);

public:
    virtual ~OutputPrinter();

    static std::unique_ptr<OutputPrinter> Create(OutputFormat format,
                                                 const std::string& outputFileName);


    void PrintRuleViolation(const std::string& ruleName,
                            Severity severity,
                            const std::string& description,
                            clang::SourceLocation location,
                            clang::SourceManager& sourceManager,
                            int lineOffset = 0);

    virtual void PrintRuleViolation(const std::string& ruleName,
                                    Severity severity,
                                    const std::string& description,
                                    const std::string& fileName,
                                    int lineNumber) = 0;

    virtual void PrintGraphEdge(const std::string& source,
                                const std::string& destination,
                                const std::string& options = "") = 0;

    virtual void Save() = 0;

protected:
    std::string GetSeverityString(Severity severity);

protected:
    const std::string m_outputFileName;
};
