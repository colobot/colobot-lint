#pragma once

#include "TokenRule.h"

class BraceInNewLineRule : public TokenRule
{
public:
    BraceInNewLineRule(Context& context);

    void HandleToken(clang::Preprocessor& pp, const clang::Token& token) override;

private:
    clang::tok::TokenKind m_previousTokenKind;
    int m_previousTokenLineNumber;
};
