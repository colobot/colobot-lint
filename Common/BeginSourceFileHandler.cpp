#include "BeginSourceFileHandler.h"

#include "Context.h"

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
            std::cerr << " (identified as fake header file; header suffix " << m_context.actualHeaderFileSuffix << ")";
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
    auto pos = filename.find(g_fake_header_dir_prefix);
    auto suffix = filename.substr(pos + strlen(g_fake_header_dir_prefix));
    auto suffixWithoutCpp = suffix.drop_back(3); // cpp
    return suffixWithoutCpp.str() + "h";
}
