#include "OutputPrinter.h"

#include "ColobotLintConfig.h"

#include <clang/Basic/SourceLocation.h>
#include <clang/AST/ASTContext.h>


OutputPrinter::OutputPrinter(const std::string& outputFileName)
    : m_outputFileName(outputFileName)
{
    TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "", "");
    m_document.LinkEndChild(decl);

    TiXmlElement* resultsElement = new TiXmlElement("results");
    resultsElement->SetAttribute("version", "2");
    m_document.LinkEndChild(resultsElement);

    TiXmlElement* colobotLintElement = new TiXmlElement("cppcheck");
    colobotLintElement->SetAttribute("version", "colobot-lint-" COLOBOT_LINT_VERSION_STR);
    resultsElement->LinkEndChild(colobotLintElement);

    m_errorsElement = new TiXmlElement("errors");
    resultsElement->LinkEndChild(m_errorsElement);
}

void OutputPrinter::Save()
{
    if (m_outputFileName.empty())
    {
        m_document.Print();
    }
    else
    {
        m_document.SaveFile(m_outputFileName);
    }
}

void OutputPrinter::PrintRuleViolation(const std::string& ruleName,
                                       Severity severity,
                                       const std::string& description,
                                       const clang::SourceLocation& location,
                                       clang::SourceManager& sourceManager,
                                       int lineOffset)
{
    std::string fileName = sourceManager.getFilename(location).str();
    int lineNumber = sourceManager.getPresumedLineNumber(location) + lineOffset;

    PrintRuleViolation(ruleName, severity, description, fileName, lineNumber);
}

void OutputPrinter::PrintRuleViolation(const std::string& ruleName,
                                       Severity severity,
                                       const std::string& description,
                                       const std::string& fileName,
                                       int lineNumber)
{
    TiXmlElement* errorElement = new TiXmlElement("error");

    errorElement->SetAttribute("id", ruleName);
    errorElement->SetAttribute("severity", GetSeverityString(severity));
    errorElement->SetAttribute("msg", description);
    errorElement->SetAttribute("verbose", description);

    TiXmlElement* locationElement = new TiXmlElement("location");
    locationElement->SetAttribute("file", fileName);
    locationElement->SetAttribute("line", std::to_string(lineNumber));
    errorElement->LinkEndChild(locationElement);

    m_errorsElement->LinkEndChild(errorElement);
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

