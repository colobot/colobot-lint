#pragma once

#include "../Utils/OutputPrinter.h"

#include "clang/ASTMatchers/ASTMatchFinder.h"

class Rule : public clang::ast_matchers::MatchFinder::MatchCallback
{
public:
    Rule(OutputPrinter& printer)
        : m_printer(printer)
    {}

    virtual ~Rule()
    {}

protected:
    OutputPrinter& m_printer;
};
