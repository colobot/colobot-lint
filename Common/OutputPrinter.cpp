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
    PlainTextOutputPrinter(const std::string& outputFileName);

    void PrintGraphEdge(const std::string& source,
                        const std::string& destination,
                        const std::string& options = "") override;

    void PrintRuleViolation(const std::string& ruleName,
                            Severity severity,
                            const std::string& description,
                            const std::string& fileName,
                            int lineNumber) override;

    void Save() override;

private:
    std::ofstream m_outputFileStream;
    std::ostream& m_outputStream;
};

class XmlOutputPrinter : public OutputPrinter
{
public:
    XmlOutputPrinter(const std::string& outputFileName);

    void PrintGraphEdge(const std::string& source,
                        const std::string& destination,
                        const std::string& options = "") override;

    void PrintRuleViolation(const std::string& ruleName,
                            Severity severity,
                            const std::string& description,
                            const std::string& fileName,
                            int lineNumber) override;

    void Save() override;

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
    DotGraphOutputPrinter(const std::string& outputFileName);

    void PrintGraphEdge(const std::string& source,
                        const std::string& destination,
                        const std::string& options = "") override;

    void PrintRuleViolation(const std::string& ruleName,
                            Severity severity,
                            const std::string& description,
                            const std::string& fileName,
                            int lineNumber) override;

    void Save() override;

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



OutputPrinter::OutputPrinter(const std::string& outputFileName)
    : m_outputFileName(outputFileName)
{}

OutputPrinter::~OutputPrinter()
{}

std::unique_ptr<OutputPrinter> OutputPrinter::Create(OutputFormat format,
                                                     const std::string& outputFileName)
{
    if (format == OutputFormat::PlainTextReport)
        return make_unique<PlainTextOutputPrinter>(outputFileName);
    else if (format == OutputFormat::XmlReport)
        return make_unique<XmlOutputPrinter>(outputFileName);

    return make_unique<DotGraphOutputPrinter>(outputFileName);
}

void OutputPrinter::PrintRuleViolation(const std::string& ruleName,
                                       Severity severity,
                                       const std::string& description,
                                       SourceLocation location,
                                       SourceManager& sourceManager,
                                       int lineOffset)
{
    std::string fileName = GetCleanFilename(location, sourceManager);
    int lineNumber = sourceManager.getPresumedLineNumber(location) + lineOffset;

    PrintRuleViolation(ruleName, severity, description, fileName, lineNumber);
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

///////////////////////////

PlainTextOutputPrinter::PlainTextOutputPrinter(const std::string& outputFileName)
    : OutputPrinter(outputFileName),
      m_outputStream(outputFileName.empty() ? std::cout : m_outputFileStream)
{
    if (!outputFileName.empty())
    {
        m_outputFileStream.open(outputFileName.c_str());
    }
}

void PlainTextOutputPrinter::PrintRuleViolation(const std::string& ruleName,
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

void PlainTextOutputPrinter::Save()
{
}


///////////////////////////

XmlOutputPrinter::XmlOutputPrinter(const std::string& outputFileName)
    : OutputPrinter(outputFileName)
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

void XmlOutputPrinter::PrintRuleViolation(const std::string& ruleName,
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

void XmlOutputPrinter::Save()
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

DotGraphOutputPrinter::DotGraphOutputPrinter(const std::string& outputFileName)
    : OutputPrinter(outputFileName)
{}

void DotGraphOutputPrinter::PrintRuleViolation(const std::string& ruleName,
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

void DotGraphOutputPrinter::Save()
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
