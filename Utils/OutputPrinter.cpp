#include "OutputPrinter.h"

OutputPrinter::OutputPrinter(const std::string& outputFileName)
{
    m_outputFile.open(outputFileName.c_str());
}

void OutputPrinter::PrintRuleViolation(const std::string& ruleName,
                                       const std::string& violationDescription,
                                       const clang::SourceLocation& location,
                                       clang::ASTContext* context)
{
    std::string locationString = location.printToString(context->getSourceManager());

    m_outputFile << "Violation of " << ruleName << " rule:" << std::endl;
    m_outputFile << " location: " << locationString << std::endl;
    m_outputFile << " description: " << violationDescription << std::endl;
}
