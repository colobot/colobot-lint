#pragma once

#include "../Common/StringRefHash.h"

#include <clang/Lex/Preprocessor.h>

#include <boost/regex.hpp>

#include <unordered_set>

class Context;

class ExclusionZoneCommentHandler : public clang::CommentHandler
{
public:
    ExclusionZoneCommentHandler(Context& context);

    void RegisterPPCallbacks(clang::Preprocessor& pp);

    bool HandleComment(clang::Preprocessor& pp, clang::SourceRange comment) override;

    void AtEndOfMainFile();

private:
    void HandleCommentLine(llvm::StringRef commentLine, int lineNumber);
    void HandleExclusionDirective(llvm::StringRef ruleNames);
    void FlushExcludeZone(int currentLineNumber);

private:
    Context& m_context;
    const boost::regex m_excludeDirectivePattern;
    const boost::regex m_excludeEndDirectivePattern;
    int m_excludeZoneStartingLineNumber;
    std::unordered_set<llvm::StringRef> m_currentlyExcludedRules;
};
