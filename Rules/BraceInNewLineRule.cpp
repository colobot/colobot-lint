#include "BraceInNewLineRule.h"

#include "../Common/SourceLocationHelper.h"

#include "clang/Lex/Preprocessor.h"
#include "clang/Lex/Token.h"

using namespace clang;

BraceInNewLineRule::BraceInNewLineRule(Context& context)
    : TokenRule(context)
    , m_previousTokenKind(tok::eof)
    , m_previousTokenLineNumber(0)
{}

void BraceInNewLineRule::HandleToken(clang::Preprocessor& pp, const clang::Token& token)
{
    SourceLocation tokenLocation = token.getLocation();
    if (! m_context.sourceLocationHelper.IsLocationOfInterest(tokenLocation, pp.getSourceManager()))
        return;

    auto currentTokenKind = token.getKind();
    int currentTokenLineNumber = pp.getSourceManager().getSpellingLineNumber(tokenLocation);

    if (currentTokenKind == tok::l_brace)
    {
        if (m_previousTokenKind != tok::eof &&
            m_previousTokenKind != tok::l_brace &&
            !token.isAtStartOfLine())
        {
            m_context.printer.PrintRuleViolation(
                "brace placement",
                Severity::Style,
                "Opening brace should be placed only at the beginning of line",
                tokenLocation,
                pp.getSourceManager());
        }
    }

    if (m_previousTokenKind == tok::r_brace)
    {
        if (currentTokenKind != tok::r_brace && currentTokenKind != tok::semi)
        {
            if (currentTokenLineNumber == m_previousTokenLineNumber)
            {
                m_context.printer.PrintRuleViolation(
                    "brace placement",
                    Severity::Style,
                    "Closing brace should be placed only at the end of line",
                    tokenLocation,
                    pp.getSourceManager());
            }
        }
    }

    m_previousTokenKind = currentTokenKind;
    m_previousTokenLineNumber = currentTokenLineNumber;
}
