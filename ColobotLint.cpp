#include "Common/Context.h"
#include "Common/OutputPrinter.h"
#include "Common/SourceLocationHelper.h"

#include "Handlers/DiagnosticHandler.h"

#include "ActionFactories.h"

#include "ColobotLintConfig.h"

#include <llvm/Support/CommandLine.h>
#include <llvm/Config/config.h>
#include <clang/Tooling/CommonOptionsParser.h>

#include <iostream>
#include <exception>

using namespace llvm;
using namespace llvm::cl;
using namespace clang;
using namespace clang::tooling;
using namespace clang::ast_matchers;

namespace
{

OptionCategory g_colobotLintOptionCategory("colobot-lint options");

static cl::opt<std::string> g_projectRootSourceDirectory(
    Required,
    "project-root",
    desc("Full path to project source directory"),
    value_desc("directory"), cat(g_colobotLintOptionCategory));

static cl::opt<std::string> g_outputFileOpt(
    "output-file",
    desc("Where to save the XML output; if not given, write to stderr"),
    value_desc("filename"), cat(g_colobotLintOptionCategory));

static cl::opt<bool> g_verboseOpt(
    "verbose",
    desc("Whether to print verbose output"),
    init(false), cat(g_colobotLintOptionCategory));

static cl::opt<bool> g_debugOpt(
    "debug",
    desc("Whether to print even more verbose output"),
    init(false), cat(g_colobotLintOptionCategory));

static cl::list<std::string> g_onlyRule(
    "only-rule",
    desc("Run only these rule(s)"),
    cat(g_colobotLintOptionCategory)
);

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

void printColobotLintVersion()
{
    std::cout << "colobot-lint version " << COLOBOT_LINT_VERSION_STR <<  " built with LLVM " << LLVM_VERSION_STRING << std::endl;
    std::cout << "(C) 2015 Piotr Dziwinski <piotrdz@gmail.com>" << std::endl;
    std::cout << "http://colobot.info http://github.com/colobot/colobot" << std::endl;
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
    SetVersionPrinter(printColobotLintVersion);
    CommonOptionsParser optionsParser(argc, argv, g_colobotLintOptionCategory);

    ClangTool tool(optionsParser.getCompilations(),
                   optionsParser.getSourcePathList());

    std::set<std::string> rulesSelection;
    for (const auto& rule : g_onlyRule)
        rulesSelection.insert(rule);

    SourceLocationHelper sourceLocationHelper;

    OutputPrinter outputPrinter(g_outputFileOpt);

    Context context(sourceLocationHelper,
                    outputPrinter,
                    g_projectRootSourceDirectory,
                    rulesSelection,
                    g_verboseOpt,
                    g_debugOpt);
    sourceLocationHelper.SetContext(&context);

    DiagnosticHandler diagnosticHandler(context);
    tool.setDiagnosticConsumer(&diagnosticHandler);

    ColobotLintASTFrontendActionFactory factory(context);
    int retCode = tool.run(&factory);

    outputPrinter.Save();

    return retCode;
}
