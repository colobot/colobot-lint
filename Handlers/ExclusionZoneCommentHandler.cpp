#include "ExclusionZoneCommentHandler.h"

#include "../Common/Context.h"
#include "../Common/SourceLocationHelper.h"
#include "../Common/RegexHelper.h"
#include "../Common/TextHelper.h"

#include <iostream>

using namespace llvm;
using namespace clang;

class ExclusionZonePPCallbacks : public PPCallbacks
{
public:
    ExclusionZonePPCallbacks(ExclusionZoneCommentHandler& handler, SourceManager& sourceManager)
        : m_handler(handler)
    {}


    void EndOfMainFile() override
    {
        m_handler.AtEndOfMainFile();
    }

private:
    ExclusionZoneCommentHandler& m_handler;
};

/////////////

ExclusionZoneCommentHandler::ExclusionZoneCommentHandler(Context& context)
    : m_context(context),
      m_excludeDirectivePattern("@colobot-lint-exclude (\\*|([[:lower:][:upper:]]+( +[[:lower:][:upper:]]+)*))"),
      m_excludeEndDirectivePattern("@end-colobot-lint-exclude"),
      m_excludeZoneStartingLineNumber(0)
{}

void ExclusionZoneCommentHandler::RegisterPPCallbacks(Preprocessor& pp)
{
    pp.addPPCallbacks(make_unique<ExclusionZonePPCallbacks>(*this, pp.getSourceManager()));
}

bool ExclusionZoneCommentHandler::HandleComment(Preprocessor& pp, SourceRange range)
{
    SourceLocation location = range.getBegin();

    if (! m_context.sourceLocationHelper.IsLocationOfInterestIgnoringExclusionZone(location, pp.getSourceManager()))
        return false;

    StringRef commentText = Lexer::getSourceText(CharSourceRange::getCharRange(range),
                                                 pp.getSourceManager(),
                                                 pp.getLangOpts());

    int lineNumber = pp.getSourceManager().getPresumedLineNumber(location);
    int lineOffset = 0;
    while (! commentText.empty())
    {
        auto split = commentText.split('\n');
        HandleCommentLine(split.first, lineNumber + lineOffset);
        commentText = split.second;
        ++lineOffset;
    }

    return false;
}

void ExclusionZoneCommentHandler::AtEndOfMainFile()
{
    if (! m_currentlyExcludedRules.empty())
    {
        std::cerr << "Warning: unclosed @colobot-lint-exclude directive from line " << m_excludeZoneStartingLineNumber
                  << " will have no effect!" << std::endl;
    }
}

void ExclusionZoneCommentHandler::HandleCommentLine(StringRef commentLine, int lineNumber)
{
    StringRefMatchResults ruleNames;
    if (boost::regex_search(commentLine.begin(), commentLine.end(), ruleNames, m_excludeDirectivePattern))
    {
        FlushExcludeZone(lineNumber);
        HandleExclusionDirective(GetStringRefResult(ruleNames, 1, commentLine));
        m_excludeZoneStartingLineNumber = lineNumber;
    }
    else if (boost::regex_search(commentLine.begin(), commentLine.end(), m_excludeEndDirectivePattern))
    {
        FlushExcludeZone(lineNumber);
        m_currentlyExcludedRules.clear();
        m_excludeZoneStartingLineNumber = 0;
    }
}

void ExclusionZoneCommentHandler::HandleExclusionDirective(StringRef ruleNames)
{
    while (! ruleNames.empty())
    {
        auto split = ruleNames.split(' ');
        m_currentlyExcludedRules.insert(split.first);
        ruleNames = split.second;
    }
}

void ExclusionZoneCommentHandler::FlushExcludeZone(int currentLineNumber)
{
    if (m_currentlyExcludedRules.empty())
        return;

    for (int lineNumber = m_excludeZoneStartingLineNumber;
         lineNumber <= currentLineNumber;
         ++lineNumber)
    {
        for (const auto& rule : m_currentlyExcludedRules)
        {
            m_context.exclusionZones.insert(ExclusionZone{lineNumber, rule});
        }
    }
}
