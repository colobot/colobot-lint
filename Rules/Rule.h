#pragma once

#include "../Utils/OutputPrinter.h"

class Rule
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
