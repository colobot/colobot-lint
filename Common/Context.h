#pragma once

#include "Common/ExclusionZone.h"
#include "Common/FunctionDefinitionContext.h"
#include "Common/OutputPrinter.h"
#include "Common/SourceFileInfo.h"

#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>

class SourceLocationHelper;

struct Context
{
    Context(SourceLocationHelper& sourceLocationHelper,
            std::unique_ptr<OutputPrinter> printer,
            std::set<std::string> projectLocalIncludePaths,
            std::vector<std::string> licenseTemplateLines,
            std::set<std::string> rulesSelection,
            std::string generatorSelection,
            bool verbose,
            bool debug)
        : rulesSelection(std::move(rulesSelection)),
          generatorSelection(std::move(generatorSelection)),
          projectLocalIncludePaths(std::move(projectLocalIncludePaths)),
          licenseTemplateLines(std::move(licenseTemplateLines)),
          verbose(verbose),
          debug(debug),
          areWeInFakeHeaderSourceFile(false),
          sourceLocationHelper(sourceLocationHelper),
          outputPrinter(std::move(printer))
    {}

    const std::set<std::string> rulesSelection;
    const std::string generatorSelection;
    const std::set<std::string> projectLocalIncludePaths;
    const std::vector<std::string> licenseTemplateLines;
    const bool verbose;
    const bool debug;

    bool areWeInFakeHeaderSourceFile;
    std::string actualHeaderFileSuffix;
    std::unordered_set<std::string> processedFiles;

    std::unordered_set<ExclusionZone> exclusionZones;

    std::unordered_set<std::string> reportedOldStyleFunctions;

    std::unordered_set<std::string> definedFunctions;
    std::unordered_map<std::string, SourceFileInfo> undefinedFunctions;

    SourceLocationHelper& sourceLocationHelper;

    const std::unique_ptr<OutputPrinter> outputPrinter;
};
