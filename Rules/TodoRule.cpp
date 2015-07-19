#include "TodoRule.h"

#include "../Common/SourceLocationHelper.h"


using namespace clang;

TodoRule::TodoRule(Context& context)
    : ASTRule(context)
    , m_todoPattern("TODO.*$")
{}

void TodoRule::RegisterPreProcessorCallbacks(clang::CompilerInstance& compiler)
{
    compiler.getPreprocessor().addCommentHandler(this);
}

bool TodoRule::HandleComment(clang::Preprocessor& pp, clang::SourceRange range)
{
    SourceLocation location = range.getBegin();

    if (! m_context.sourceLocationHelper.IsLocationOfInterest(location, pp.getSourceManager()))
        return false;

    std::string commentText =
        Lexer::getSourceText(CharSourceRange::getCharRange(range),
                             pp.getSourceManager(), pp.getLangOpts());

    std::vector<std::string> lines = SplitLines(commentText);
    for (const auto& commentLine : lines)
    {
        std::smatch match;
        if (std::regex_search(commentLine, match, m_todoPattern))
        {
            std::string todoText = match[0];
            m_context.printer.PrintRuleViolation("TODO comment",
                                                Severity::Information,
                                                todoText,
                                                location,
                                                pp.getSourceManager());
        }
    }

    return false;
}

std::vector<std::string> TodoRule::SplitLines(const std::string& text)
{
    std::vector<std::string> lines;
    size_t pos = 0;
    while (pos < text.length())
    {
        size_t newlinePos = text.find('\n', pos);
        if (newlinePos != text.npos)
        {
            lines.push_back(text.substr(pos, newlinePos - pos));
            pos = newlinePos+1;
        }
        else
        {
            lines.push_back(text.substr(pos));
            break;
        }
    }
    return lines;
}
