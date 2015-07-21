#pragma once

#include <string>

class SourceLocationHelper;
class OutputPrinter;

struct Context
{
    Context(SourceLocationHelper& _sourceLocationHelper,
            OutputPrinter& _printer,
            bool _verbose)
        : areWeInFakeHeaderSourceFile(false)
        , sourceLocationHelper(_sourceLocationHelper)
        , printer(_printer)
        , verbose(_verbose)
    {}

    bool areWeInFakeHeaderSourceFile;
    std::string actualHeaderFileSuffix;

    SourceLocationHelper& sourceLocationHelper;

    OutputPrinter& printer;

    const bool verbose;
};
