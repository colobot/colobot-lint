#pragma once

#include "OutputPrinter.h"

struct Context
{
    Context(OutputPrinter& _printer, bool _verbose)
        : printer(_printer)
        , verbose(_verbose)
    {}

    OutputPrinter& printer;
    const bool verbose;
};
