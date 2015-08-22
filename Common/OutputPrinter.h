#pragma once

#include "Common/Severity.h"

#include <memory>
#include <string>
#include <vector>

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

struct OutputFilter
{
    std::string fileName;
    int startLineNumber = 0;
    int endLineNumber = 0;
};

class OutputPrinter
{
protected:
    OutputPrinter(const std::string& outputFileName,
                  std::vector<OutputFilter> outputFilters);

public:
    virtual ~OutputPrinter();

    static std::unique_ptr<OutputPrinter> Create(OutputFormat format,
                                                 const std::string& outputFileName,
                                                 std::vector<OutputFilter> outputFilters);


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

    virtual void PrintGraphEdge(const std::string& source,
                                const std::string& destination,
                                const std::string& options = "") = 0;

    virtual void Save() = 0;

protected:
    virtual void PrintRuleViolationImpl(const std::string& ruleName,
                                        Severity severity,
                                        const std::string& description,
                                        const std::string& fileName,
                                        int lineNumber) = 0;
    bool ShouldPrintLine(const std::string& fileName, int lineNumber);
    std::string GetSeverityString(Severity severity);

protected:
    const std::string m_outputFileName;
    const std::vector<OutputFilter> m_outputFilters;
};
