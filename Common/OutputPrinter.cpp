#include "Common/OutputPrinter.h"

#include "ColobotLintConfig.h"

#include "Common/FilenameHelper.h"

#include <clang/Basic/SourceLocation.h>
#include <clang/AST/ASTContext.h>
#include <llvm/ADT/STLExtras.h>

using namespace clang;
using namespace llvm;


OutputPrinter::OutputPrinter(const std::string& outputFileName)
    : m_outputFileName(outputFileName)
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

void OutputPrinter::Save()
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

void OutputPrinter::PrintRuleViolation(const std::string& ruleName,
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

