#pragma once

#include "Rule.h"

class TokenRule : public Rule
{
public:
    TokenRule(Context& context)
        : Rule(context)
    {}

    virtual void HandleToken(clang::Preprocessor& pp, const clang::Token& token) = 0;
};
