#include "OutputPrinter.h"

OutputPrinter::OutputPrinter(const std::string& outputFileName)
    : m_outputFileName(outputFileName)
{
    TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "", "");
    m_document.LinkEndChild(decl);

    m_resultsElement = new TiXmlElement("results");
}

void OutputPrinter::Save()
{
    m_document.LinkEndChild(m_resultsElement);
    m_document.SaveFile(m_outputFileName);
}

void OutputPrinter::PrintRuleViolation(const std::string& ruleName,
                                       Severity severity,
                                       const std::string& description,
                                       const clang::SourceLocation& location,
                                       clang::SourceManager& sourceManager)
{
    TiXmlElement* errorElement = new TiXmlElement("error");

    errorElement->SetAttribute("file", sourceManager.getFilename(location).str());
    errorElement->SetAttribute("line", std::to_string(sourceManager.getSpellingLineNumber(location)));
    errorElement->SetAttribute("id", ruleName);
    errorElement->SetAttribute("severity", GetSeverityString(severity));
    errorElement->SetAttribute("msg", description);

    m_resultsElement->LinkEndChild(errorElement);
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

