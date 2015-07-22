#pragma once

#include <set>
#include <string>

class SourceLocationHelper;
class OutputPrinter;

struct Context
{
    Context(SourceLocationHelper& _sourceLocationHelper,
            OutputPrinter& _printer,
            const std::set<std::string>& _rulesSelection,
            bool _verbose)
        : areWeInFakeHeaderSourceFile(false)
        , sourceLocationHelper(_sourceLocationHelper)
        , printer(_printer)
        , rulesSelection(_rulesSelection)
        , verbose(_verbose)
    {}

    bool areWeInFakeHeaderSourceFile;
    std::string actualHeaderFileSuffix;

    SourceLocationHelper& sourceLocationHelper;

    OutputPrinter& printer;

    const std::set<std::string> rulesSelection;

    const bool verbose;
};
