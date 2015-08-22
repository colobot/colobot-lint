#pragma once

#include "Common/ExclusionZone.h"

#include <set>
#include <string>
#include <unordered_set>

class SourceLocationHelper;
class OutputPrinter;

struct Context
{
    Context(SourceLocationHelper& sourceLocationHelper,
            OutputPrinter& printer,
            std::set<std::string>&& projectLocalIncludePaths,
            std::vector<std::string>&& licenseTemplateLines,
            std::set<std::string>&& rulesSelection,
            const std::string& generatorSelection,
            bool verbose,
            bool debug)
        : rulesSelection(rulesSelection),
          generatorSelection(generatorSelection),
          projectLocalIncludePaths(projectLocalIncludePaths),
          licenseTemplateLines(licenseTemplateLines),
          verbose(verbose),
          debug(debug),
          areWeInFakeHeaderSourceFile(false),
          sourceLocationHelper(sourceLocationHelper),
          printer(printer)
    {}

    const std::set<std::string> rulesSelection;
    const std::string generatorSelection;
    const std::set<std::string> projectLocalIncludePaths;
    const std::vector<std::string> licenseTemplateLines;
    const bool verbose;
    const bool debug;

    bool areWeInFakeHeaderSourceFile;
    std::string actualHeaderFileSuffix;

    std::unordered_set<ExclusionZone> exclusionZones;

    std::unordered_set<std::string> reportedOldStyleFunctions;

    SourceLocationHelper& sourceLocationHelper;

    OutputPrinter& printer;
};
