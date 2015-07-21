#pragma once

#include <string>

struct Context;

namespace clang
{
class CompilerInstance;
}

namespace llvm
{
class StringRef;
}

class BeginSourceFileHandler
{
public:
    BeginSourceFileHandler(Context& context);

    void BeginSourceFileAction(clang::CompilerInstance& ci, llvm::StringRef filename);

private:
    bool IsFakeHeaderSource(llvm::StringRef filename);
    std::string GetActualHeaderFileSuffix(llvm::StringRef filename);

    Context& m_context;
};
