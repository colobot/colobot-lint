#include "Common/OutputPrinter.h"

#include "ColobotLintConfig.h"

#include "Common/FilenameHelper.h"

#include <clang/Basic/SourceLocation.h>
#include <clang/AST/ASTContext.h>
#include <llvm/ADT/STLExtras.h>

#include <tinyxml.h>

#include <cassert>
#include <fstream>
#include <unordered_set>

using namespace clang;
using namespace llvm;

namespace
{

class PlainTextOutputPrinter : public OutputPrinter
{
public:
    PlainTextOutputPrinter(const std::string& outputFileName,
                           std::vector<OutputFilter> outputFilters);

    void PrintGraphEdge(const std::string& source,
                        const std::string& destination,
                        const std::string& options = "") override;

protected:
    void PrintRuleViolationImpl(const std::string& ruleName,
                                Severity severity,
                                const std::string& description,
                                const std::string& fileName,
                                int lineNumber) override;

    void SaveImpl() override;

private:
    std::ofstream m_outputFileStream;
    std::ostream& m_outputStream;
};

class XmlOutputPrinter : public OutputPrinter
{
public:
    XmlOutputPrinter(const std::string& outputFileName,
                     std::vector<OutputFilter> outputFilters);

    void PrintGraphEdge(const std::string& source,
                        const std::string& destination,
                        const std::string& options = "") override;

protected:
    void PrintRuleViolationImpl(const std::string& ruleName,
                                Severity severity,
                                const std::string& description,
                                const std::string& fileName,
                                int lineNumber) override;

    void SaveImpl() override;

private:
    void Init();

private:
    TiXmlDocument m_document;
    std::unique_ptr<TiXmlElement> m_resultsElement;
    std::unique_ptr<TiXmlElement> m_errorsElement;
};

class DotGraphOutputPrinter : public OutputPrinter
{
public:
    DotGraphOutputPrinter(const std::string& outputFileName,
                          std::vector<OutputFilter> outputFilters);

    void PrintGraphEdge(const std::string& source,
                        const std::string& destination,
                        const std::string& options = "") override;

protected:
    void PrintRuleViolationImpl(const std::string& ruleName,
                                Severity severity,
                                const std::string& description,
                                const std::string& fileName,
                                int lineNumber) override;

    void SaveImpl() override;

private:
    void Save(std::ostream& ouputStream);

private:
    struct DotGraphEdge
    {
        std::string source;
        std::string target;
        std::string options;

        bool operator==(const DotGraphEdge& other) const
        {
            return source == other.source &&
                target == other.target &&
                options == other.options;
        }
    };

    struct DotGraphEdgeHash
    {
        std::size_t operator()(const DotGraphEdge& edge) const
        {
            auto sourceHash = std::hash<std::string>()(edge.source);
            auto targetHash = std::hash<std::string>()(edge.target);
            auto optionsHash = std::hash<std::string>()(edge.options);
            return (sourceHash << 12) | (targetHash << 6) | optionsHash;
        }
    };

    std::unordered_set<DotGraphEdge, DotGraphEdgeHash> m_graphEdges;
};


} // anonymous namespace


struct OutputPrinter::RuleViolationInfo
{
    std::string ruleName;
    Severity severity;
    std::string description;
    std::string fileName;
    int lineNumber;

    RuleViolationInfo(std::string ruleName,
                      Severity severity,
                      std::string description,
                      std::string fileName,
                      int lineNumber)
        : ruleName(ruleName),
          severity(severity),
          description(description),
          fileName(fileName),
          lineNumber(lineNumber)
    {}
};


OutputPrinter::OutputPrinter(const std::string& outputFileName,
                             std::vector<OutputFilter> outputFilters)
    : m_outputFileName(outputFileName),
      m_outputFilters(std::move(outputFilters))
{}

OutputPrinter::~OutputPrinter()
{}

std::unique_ptr<OutputPrinter> OutputPrinter::Create(OutputFormat format,
                                                     const std::string& outputFileName,
                                                     std::vector<OutputFilter> outputFilters)
{
    if (format == OutputFormat::PlainTextReport)
        return make_unique<PlainTextOutputPrinter>(outputFileName, std::move(outputFilters));
    else if (format == OutputFormat::XmlReport)
        return make_unique<XmlOutputPrinter>(outputFileName, std::move(outputFilters));

    return make_unique<DotGraphOutputPrinter>(outputFileName, std::move(outputFilters));
}

void OutputPrinter::PrintRuleViolation(const std::string& ruleName,
                                       Severity severity,
                                       const std::string& description,
                                       SourceLocation location,
                                       SourceManager& sourceManager,
                                       int lineOffset,
                                       bool tentative)
{
    std::string fileName = GetCleanFilename(location, sourceManager);
    int lineNumber = sourceManager.getPresumedLineNumber(location) + lineOffset;

    PrintRuleViolation(ruleName, severity, description, fileName, lineNumber, tentative);
}

void OutputPrinter::PrintRuleViolation(const std::string& ruleName,
                                       Severity severity,
                                       const std::string& description,
                                       const std::string& fileName,
                                       int lineNumber,
                                       bool tentative)
{
    if (ShouldPrintLine(fileName, lineNumber))
    {
        if (tentative)
        {
            m_tentativeViolations.emplace_back(ruleName, severity, description, fileName, lineNumber);
        }
        else
        {
            PrintRuleViolationImpl(ruleName, severity, description, fileName, lineNumber);
        }
    }
}

void OutputPrinter::ClearTentativeViolations()
{
    m_tentativeViolations.clear();
}

bool OutputPrinter::ShouldPrintLine(const std::string& fileName, int lineNumber)
{
    if (m_outputFilters.empty())
        return true;

    for (const auto& filter : m_outputFilters)
    {
        if (StringRef(fileName).endswith(StringRef(filter.fileName)) &&
            lineNumber >= filter.startLineNumber &&
            lineNumber <= filter.endLineNumber)
        {
            return true;
        }
    }

    return false;
}

std::string OutputPrinter::GetSeverityString(Severity severity)
{
    std::string str;

    switch (severity)
    {
        case Severity::Style:
            str = "style";
            break;

        case Severity::Error:
            str = "error";
            break;

        case Severity::Warning:
            str = "warning";
            break;

        case Severity::Information:
            str = "information";
            break;
    }

    return str;
}

void OutputPrinter::Save()
{
    for (const auto& violationInfo : m_tentativeViolations)
    {
        PrintRuleViolation(violationInfo.ruleName,
                           violationInfo.severity,
                           violationInfo.description,
                           violationInfo.fileName,
                           violationInfo.lineNumber);
    }

    m_tentativeViolations.clear();

    SaveImpl();
}

///////////////////////////

PlainTextOutputPrinter::PlainTextOutputPrinter(const std::string& outputFileName,
                                               std::vector<OutputFilter> outputFilters)
    : OutputPrinter(outputFileName, std::move(outputFilters)),
      m_outputStream(outputFileName.empty() ? std::cout : m_outputFileStream)
{
    if (!outputFileName.empty())
    {
        m_outputFileStream.open(outputFileName.c_str());
    }
}

void PlainTextOutputPrinter::PrintRuleViolationImpl(const std::string& ruleName,
                                                    Severity severity,
                                                    const std::string& description,
                                                    const std::string& fileName,
                                                    int lineNumber)
{
    m_outputStream << "[" << GetSeverityString(severity) << "]" << " "
                   << "[" << ruleName << "]" << " "
                   << fileName << ":" << lineNumber << " "
                   << description << std::endl;
}

void PlainTextOutputPrinter::PrintGraphEdge(const std::string& source,
                                            const std::string& destination,
                                            const std::string& options)
{
    assert(false && "Not implemented");
}

void PlainTextOutputPrinter::SaveImpl()
{
}


///////////////////////////

XmlOutputPrinter::XmlOutputPrinter(const std::string& outputFileName,
                                   std::vector<OutputFilter> outputFilters)
    : OutputPrinter(outputFileName, std::move(outputFilters))
{
    Init();
}

void XmlOutputPrinter::Init()
{
    auto decl = make_unique<TiXmlDeclaration>("1.0", "", "");
    m_document.LinkEndChild(decl.release());

    m_resultsElement = make_unique<TiXmlElement>("results");
    m_resultsElement->SetAttribute("version", "2");

    auto colobotLintElement = make_unique<TiXmlElement>("cppcheck");
    colobotLintElement->SetAttribute("version", "colobot-lint-" COLOBOT_LINT_VERSION_STR);
    m_resultsElement->LinkEndChild(colobotLintElement.release());

    m_errorsElement = make_unique<TiXmlElement>("errors");
}

void XmlOutputPrinter::PrintRuleViolationImpl(const std::string& ruleName,
                                              Severity severity,
                                              const std::string& description,
                                              const std::string& fileName,
                                              int lineNumber)
{
    auto errorElement = make_unique<TiXmlElement>("error");

    errorElement->SetAttribute("id", ruleName);
    errorElement->SetAttribute("severity", GetSeverityString(severity));
    errorElement->SetAttribute("msg", description);
    errorElement->SetAttribute("verbose", description);

    auto locationElement = make_unique<TiXmlElement>("location");
    locationElement->SetAttribute("file", fileName);
    locationElement->SetAttribute("line", std::to_string(lineNumber));
    errorElement->LinkEndChild(locationElement.release());

    m_errorsElement->LinkEndChild(errorElement.release());
}

void XmlOutputPrinter::PrintGraphEdge(const std::string& source,
                                      const std::string& destination,
                                      const std::string& options)
{
    assert(false && "Not implemented");
}

void XmlOutputPrinter::SaveImpl()
{
    m_resultsElement->LinkEndChild(m_errorsElement.release());
    m_document.LinkEndChild(m_resultsElement.release());

    if (m_outputFileName.empty())
    {
        m_document.Print();
    }
    else
    {
        m_document.SaveFile(m_outputFileName);
    }
}

///////////////////////////

DotGraphOutputPrinter::DotGraphOutputPrinter(const std::string& outputFileName,
                                             std::vector<OutputFilter> outputFilters)
    : OutputPrinter(outputFileName, std::move(outputFilters))
{}

void DotGraphOutputPrinter::PrintRuleViolationImpl(const std::string& ruleName,
                                                   Severity severity,
                                                   const std::string& description,
                                                   const std::string& fileName,
                                                   int lineNumber)
{
    assert(false && "Not implemented");
}

void DotGraphOutputPrinter::PrintGraphEdge(const std::string& source,
                                           const std::string& destination,
                                           const std::string& options)
{
    m_graphEdges.insert(DotGraphEdge{source, destination, options});
}

void DotGraphOutputPrinter::SaveImpl()
{
    if (m_outputFileName.empty())
    {
        Save(std::cout);
    }
    else
    {
        std::ofstream outputFileStream(m_outputFileName.c_str());
        Save(outputFileStream);
    }
}

void DotGraphOutputPrinter::Save(std::ostream& outputStream)
{
    outputStream << "digraph G {\n";
    for (const auto& edge : m_graphEdges)
    {
        outputStream << "  \"" << edge.source << "\" -> \"" << edge.target << "\" " << edge.options << "\n";
    }
    outputStream << "}\n";
}
