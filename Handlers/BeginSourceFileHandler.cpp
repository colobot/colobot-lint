#include "Handlers/BeginSourceFileHandler.h"

#include "Common/Context.h"
#include "Common/OutputPrinter.h"

#include <llvm/ADT/StringRef.h>

#include <boost/regex.hpp>

#include <iostream>

BeginSourceFileHandler::BeginSourceFileHandler(Context& context)
    : m_context(context)
{}

bool BeginSourceFileHandler::BeginSourceFileAction(clang::CompilerInstance&, llvm::StringRef filename)
{
    auto filenameStr = filename.str();

    if (m_context.processedFiles.count(filenameStr) > 0)
    {
        if (m_context.verbose)
        {
            std::cerr << "Skipping " << filenameStr << " [already processed]" << std::endl;
        }
        return false;
    }

    if (m_context.verbose)
    {
        std::cerr << "Processing " << filenameStr;
    }

    if (IsFakeHeaderSource(filename))
    {
        m_context.areWeInFakeHeaderSourceFile = true;
        m_context.actualHeaderFileSuffix = GetActualHeaderFileSuffix(filename);

        if (m_context.verbose)
        {
            std::cerr << " [fake header mode]";
            if (m_context.debug)
            {
                std::cerr << std::endl;
                std::cerr << "Header suffix: " << m_context.actualHeaderFileSuffix;
            }
        }
    }
    else
    {
        m_context.areWeInFakeHeaderSourceFile = false;
        m_context.actualHeaderFileSuffix = "";
    }

    if (m_context.verbose)
    {
        std::cerr << std::endl;
    }

    m_context.processedFiles.insert(filenameStr);
    return true;
}

namespace
{
    const char* const FAKE_HEADER_DIR_PREFIX = "fake_header_sources/";
} // anonymous namespace

bool BeginSourceFileHandler::IsFakeHeaderSource(llvm::StringRef filename)
{
    return filename.find(FAKE_HEADER_DIR_PREFIX) != filename.npos;
}

std::string BeginSourceFileHandler::GetActualHeaderFileSuffix(llvm::StringRef filename)
{
    boost::regex searchPattern(std::string(FAKE_HEADER_DIR_PREFIX) + "(.*?)\\.cpp$");

    boost::smatch match;
    std::string filenameStr = filename.str();
    if (!boost::regex_search(filenameStr, match, searchPattern))
    {
        std::cerr << "Failed to match expected fake source file pattern!" << std::endl;
        return "<not found>"; // should not match anything
    }

    return std::string(match[1]) + ".h";
}
