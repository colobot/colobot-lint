#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

#include "llvm/Support/CommandLine.h"

#include "Rules/NakedDeleteRule.h"

#include "Utils/OutputPrinter.h"

#include <memory>

using namespace llvm;
using namespace llvm::cl;
using namespace clang;
using namespace clang::tooling;
using namespace clang::ast_matchers;

int main(int argc, const char **argv)
{
    OptionCategory colobotLintCategory("colobot-lint options");
    extrahelp commonHelp(CommonOptionsParser::HelpMessage);
    extrahelp moreHelp("Colobot lint tool...\n");
    CommonOptionsParser optionsParser(argc, argv, colobotLintCategory);

    ClangTool tool(optionsParser.getCompilations(),
                   optionsParser.getSourcePathList());

    MatchFinder finder;

    OutputPrinter printer("colobot_lint_report.txt");

    std::vector<std::unique_ptr<Rule>> rules;
    rules.push_back(make_unique<NakedDeleteRule>(finder, printer));

    return tool.run(newFrontendActionFactory(&finder).get());
}
