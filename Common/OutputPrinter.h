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

namespace llvm
{
class StringRef;
} // namespace clang

class SourceLocationHelper;

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
                  std::vector<OutputFilter> outputFilters,
                  SourceLocationHelper& sourceLocationHelper);

public:
    virtual ~OutputPrinter();

    static std::unique_ptr<OutputPrinter> Create(OutputFormat format,
                                                 const std::string& outputFileName,
                                                 std::vector<OutputFilter> outputFilters,
                                                 SourceLocationHelper& sourceLocationHelper);


    void PrintRuleViolation(llvm::StringRef ruleName,
                            Severity severity,
                            const std::string& description,
                            clang::SourceLocation location,
                            clang::SourceManager& sourceManager,
                            int lineOffset = 0,
                            bool tentative = false);

    void PrintRuleViolation(llvm::StringRef ruleName,
                            Severity severity,
                            const std::string& description,
                            llvm::StringRef fileName,
                            int lineNumber,
                            bool tentative = false);

    virtual void PrintGraphEdge(const std::string& source,
                                const std::string& destination,
                                const std::string& options = "") = 0;

    void ClearTentativeViolations();

    void Save();

protected:
    virtual void PrintRuleViolationImpl(llvm::StringRef ruleName,
                                        Severity severity,
                                        const std::string& description,
                                        llvm::StringRef fileName,
                                        int lineNumber) = 0;
    virtual void SaveImpl() = 0;
    bool ShouldPrintLine(llvm::StringRef fileName, int lineNumber);
    std::string GetSeverityString(Severity severity);

protected:
    const std::string m_outputFileName;
    const std::vector<OutputFilter> m_outputFilters;
    struct RuleViolationInfo;
    std::vector<RuleViolationInfo> m_tentativeViolations;
    SourceLocationHelper& m_sourceLocationHelper;
};
