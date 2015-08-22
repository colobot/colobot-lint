#include "Rules/TodoRule.h"

#include "Common/Context.h"
#include "Common/OutputPrinter.h"
#include "Common/RegexHelper.h"
#include "Common/SourceLocationHelper.h"

#include <clang/Frontend/CompilerInstance.h>

using namespace llvm;
using namespace clang;

TodoRule::TodoRule(Context& context)
    : ASTCallbackRule(context),
      m_todoPattern("(TODO.*?)(\\s*)?(\\*/)?$")
{}

void TodoRule::RegisterPreProcessorCallbacks(CompilerInstance& compiler)
{
    compiler.getPreprocessor().addCommentHandler(this);
}

bool TodoRule::HandleComment(Preprocessor& pp, SourceRange range)
{
    SourceLocation location = range.getBegin();

    SourceManager& sourceManager = pp.getSourceManager();

    if (! m_context.sourceLocationHelper.IsLocationOfInterest(GetName(), location, sourceManager))
        return false;

    StringRef commentText = Lexer::getSourceText(CharSourceRange::getCharRange(range),
                                                 pp.getSourceManager(),
                                                 pp.getLangOpts());

    int lineOffset = 0;
    while (! commentText.empty())
    {
        auto split = commentText.split('\n');
        StringRef commentLine = split.first;
        commentText = split.second;

        StringRefMatchResults todoText;
        if (boost::regex_search(commentLine.begin(), commentLine.end(), todoText, m_todoPattern))
        {
            m_context.outputPrinter->PrintRuleViolation("TODO comment",
                                                Severity::Information,
                                                GetStringRefResult(todoText, 1, commentLine).str(),
                                                location,
                                                sourceManager,
                                                lineOffset);
        }
        ++lineOffset;
    }

    return false;
}
