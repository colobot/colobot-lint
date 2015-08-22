#include "ActionFactories.h"

#include "ColobotLintConfig.h"

#include "Common/Context.h"
#include "Common/OutputPrinter.h"
#include "Common/SourceLocationHelper.h"

#include "Handlers/DiagnosticHandler.h"

#include <llvm/Support/CommandLine.h>
#include <llvm/Config/config.h>
#include <clang/Tooling/CommonOptionsParser.h>

#include <iostream>
#include <fstream>
#include <exception>

#include <boost/optional.hpp>

using namespace llvm;
using namespace llvm::cl;
using namespace clang;
using namespace clang::tooling;
using namespace clang::ast_matchers;

namespace
{

OptionCategory g_colobotLintOptionCategory("colobot-lint options");

static cl::list<std::string> g_projectLocalIncludePathsOpt(
    "project-local-include-path",
    desc("Search path(s) to project local include files"),
    value_desc("path"),
    cat(g_colobotLintOptionCategory));

static cl::opt<std::string> g_licenseTemplateFileOpt(
    "license-template-file",
    desc("Template of license that should be present at the beginning of project source files"),
    value_desc("filename"),
    cat(g_colobotLintOptionCategory));

static cl::opt<std::string> g_outputFormat(
    "output-format",
    desc("Format of output: plain text or XML"),
    value_desc("plain|xml"),
    init("plain"),
    cat(g_colobotLintOptionCategory));

static cl::opt<std::string> g_outputFileOpt(
    "output-file",
    desc("Where to save the XML output; if not given, write to stderr"),
    value_desc("filename"),
    cat(g_colobotLintOptionCategory));

static cl::opt<bool> g_verboseOpt(
    "verbose",
    desc("Whether to print verbose output"),
    init(false),
    cat(g_colobotLintOptionCategory));

static cl::opt<bool> g_debugOpt(
    "debug",
    desc("Whether to print even more verbose output"),
    init(false),
    cat(g_colobotLintOptionCategory));

static cl::list<std::string> g_ruleSelectionOpt(
    "only-rule",
    desc("Run only these rule(s)"),
    value_desc("rule-class"),
    cat(g_colobotLintOptionCategory));

static cl::opt<std::string> g_generatorSelectionOpt(
    "generate-graph",
    desc("If used, don't run rule checks, but instead generate given graph in dot format"),
    value_desc("graph-type"),
    cat(g_colobotLintOptionCategory));

extrahelp g_moreHelp(
    "Colobot-lint runs just like any other tool based on Clang's libtooling.\n"
    "\n"
    "It requires that compilation database is generated for the project, that is to say CMake\n"
    "has been executed with -DCMAKE_EXPORT_COMPILE_COMMANDS=1).\n"
    "It then uses information from this file to run Clang's code parser with the exact same compile\n"
    "options as during normal build, giving it the same \"view\" of code as the compiler.\n"
    "\n"
    "When Clang code parser generates AST from input source, we can run various rules on them,\n"
    "checking the code for coding style violations.\n"
    "\n"
    "Additionally, normal build diagnostics (i.e. compile warnings and errors) are reported\n"
    "alongside our custom rules.\n"
    "\n"
    "Output is saved to XML file in a cppcheck compatible format in order to be usable with Jenkins\n"
    "cppcheck plugin.\n"
);

void PrintColobotLintVersion()
{
    std::cout << "colobot-lint version " << COLOBOT_LINT_VERSION_STR <<  " built with LLVM " << LLVM_VERSION_STRING << std::endl;
    std::cout << "(C) 2015 Piotr Dziwinski <piotrdz@gmail.com>" << std::endl;
    std::cout << "http://colobot.info http://github.com/colobot/colobot" << std::endl;
}

boost::optional<std::vector<std::string>> ReadLicenseTemplateFile(const std::string& licenseTemplateFileName)
{
    if (licenseTemplateFileName.empty())
        return std::vector<std::string>({});

    std::vector<std::string> licenseFileLines;
    std::ifstream str;
    str.open(licenseTemplateFileName.c_str());
    if (!str.good())
    {
        std::cerr << "Could not load license template file!" << std::endl;
        return boost::none;
    }

    while (!str.eof())
    {
        std::string line;
        std::getline(str, line);
        licenseFileLines.push_back(std::move(line));
    }

    return licenseFileLines;
}

boost::optional<OutputFormat> ParseOutputFormat(const std::string& outputFormat,
                                                const std::string& generatorSelection)
{
    if (!generatorSelection.empty())
        return OutputFormat::DotGraph;


    if (outputFormat == "xml")
        return OutputFormat::XmlReport;
    else if (outputFormat == "plain")
        return OutputFormat::PlainTextReport;

    std::cerr << "Invalid output format!" << std::endl;
    return boost::none;
}

struct ParsedOptions
{
    bool debug = {};
    bool verbose = {};
    std::set<std::string> rulesSelection = {};
    std::string generatorSelection = {};
    std::set<std::string> projectLocalIncludePaths = {};
    std::string outputFile = {};
    OutputFormat outputFormat = {};
    std::vector<std::string> licenseTemplateLines = {};
};

boost::optional<ParsedOptions> ParseOptions()
{
    ParsedOptions parsedOptions;

    parsedOptions.debug = g_debugOpt;
    parsedOptions.verbose = g_verboseOpt;

    for (const auto& rule : g_ruleSelectionOpt)
        parsedOptions.rulesSelection.insert(rule);

    for (const auto& path : g_projectLocalIncludePathsOpt)
        parsedOptions.projectLocalIncludePaths.insert(path);

    parsedOptions.generatorSelection = g_generatorSelectionOpt;

    parsedOptions.outputFile = g_outputFileOpt;

    auto outputFormat = ParseOutputFormat(g_outputFormat, parsedOptions.generatorSelection);
    if (outputFormat == boost::none)
        return boost::none;

    parsedOptions.outputFormat = *outputFormat;

    auto licenseTemplateLines = ReadLicenseTemplateFile(g_licenseTemplateFileOpt);
    if (licenseTemplateLines == boost::none)
        return boost::none;

    parsedOptions.licenseTemplateLines = std::move(*licenseTemplateLines);

    return parsedOptions;
}

} // anonymous namespace

namespace boost
{
    void throw_exception(std::exception const& e)
    {
        std::cerr << "exception from boost: " << e.what() << std::endl;
        std::terminate();
    }
} // namespace boost


int main(int argc, const char **argv)
{
    SetVersionPrinter(PrintColobotLintVersion);
    CommonOptionsParser optionsParser(argc, argv, g_colobotLintOptionCategory);

    ClangTool tool(optionsParser.getCompilations(),
                   optionsParser.getSourcePathList());

    auto parsedOptions = ParseOptions();
    if (parsedOptions == boost::none)
        return 1;

    SourceLocationHelper sourceLocationHelper;

    Context context(sourceLocationHelper,
                    OutputPrinter::Create(parsedOptions->outputFormat,
                                          parsedOptions->outputFile),
                    std::move(parsedOptions->projectLocalIncludePaths),
                    std::move(parsedOptions->licenseTemplateLines),
                    std::move(parsedOptions->rulesSelection),
                    parsedOptions->generatorSelection,
                    parsedOptions->verbose,
                    parsedOptions->debug);
    sourceLocationHelper.SetContext(&context);

    DiagnosticHandler diagnosticHandler(context);
    tool.setDiagnosticConsumer(&diagnosticHandler);

    ColobotLintASTFrontendActionFactory factory(context);
    int retCode = tool.run(&factory);

    context.outputPrinter->Save();

    return retCode;
}
