#pragma once

#include "OutputPrinter.h"

struct Context
{
    Context(OutputPrinter& _printer)
        : printer(_printer) {}

    OutputPrinter& printer;
};
