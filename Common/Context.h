#pragma once

#include <set>
#include <string>
#include <unordered_set>

class SourceLocationHelper;
class OutputPrinter;

struct Context
{
    Context(SourceLocationHelper& _sourceLocationHelper,
            OutputPrinter& _printer,
            const std::set<std::string>& _rulesSelection,
            bool _verbose,
            bool _debug)
        : areWeInFakeHeaderSourceFile(false),
          sourceLocationHelper(_sourceLocationHelper),
          printer(_printer),
          rulesSelection(_rulesSelection),
          verbose(_verbose),
          debug(_debug)
    {}

    bool areWeInFakeHeaderSourceFile;
    std::string actualHeaderFileSuffix;

    std::unordered_set<std::string> reportedOldStyleFunctions;

    SourceLocationHelper& sourceLocationHelper;

    OutputPrinter& printer;

    const std::set<std::string> rulesSelection;

    const bool verbose;
    const bool debug;
};
