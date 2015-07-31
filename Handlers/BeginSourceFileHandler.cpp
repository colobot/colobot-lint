#include "Handlers/BeginSourceFileHandler.h"

#include "Common/Context.h"
#include "Common/OutputPrinter.h"

#include <llvm/ADT/StringRef.h>

#include <boost/regex.hpp>

BeginSourceFileHandler::BeginSourceFileHandler(Context& context)
    : m_context(context)
{}

void BeginSourceFileHandler::BeginSourceFileAction(clang::CompilerInstance&, llvm::StringRef filename)
{
    if (m_context.verbose)
    {
        std::cerr << "Processing " << filename.str();
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
}

namespace
{
    const char* g_fake_header_dir_prefix = "fake_header_sources/";
}

bool BeginSourceFileHandler::IsFakeHeaderSource(llvm::StringRef filename)
{
    return filename.find(g_fake_header_dir_prefix) != filename.npos;
}

std::string BeginSourceFileHandler::GetActualHeaderFileSuffix(llvm::StringRef filename)
{
    boost::regex searchPattern(std::string(g_fake_header_dir_prefix) + "(.*?)\\.cpp$");

    boost::smatch match;
    std::string filenameStr = filename.str();
    if (!boost::regex_search(filenameStr, match, searchPattern))
    {
        std::cerr << "Failed to match expected fake source file pattern!" << std::endl;
        return "<not found>"; // should not match anything
    }

    return std::string(match[1]) + ".h";
}
