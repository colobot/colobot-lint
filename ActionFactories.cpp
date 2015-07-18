#include "ActionFactories.h"

#include "Rules/RulesFactory.h"

#include "llvm/ADT/STLExtras.h"

using namespace llvm;
using namespace clang;
using namespace clang::ast_matchers;

ColobotLintActionFactory::ColobotLintActionFactory(Context& context)
    : m_context(context)
{}

FrontendAction* ColobotLintActionFactory::create()
{
    return new ColobotLintFrontendAction(m_context);
}

///////////////////////////

ColobotLintFrontendAction::ColobotLintFrontendAction(Context& context)
    : m_context(context)
{}

bool ColobotLintFrontendAction::BeginSourceFileAction(CompilerInstance& ci, StringRef filename)
{
    if (m_context.verbose)
    {
        std::cerr << "Processing " << filename.str() << std::endl;
    }
    return clang::FrontendAction::BeginSourceFileAction(ci, filename);
}

std::unique_ptr<ASTConsumer> ColobotLintFrontendAction::CreateASTConsumer(CompilerInstance& compiler,
                                                                          StringRef file)
{
    std::vector<std::unique_ptr<ASTConsumer>> consumers;

    std::unique_ptr<MatchFinder> finder = make_unique<MatchFinder>();

    std::vector<std::unique_ptr<Rule>> rules = CreateRules(m_context);
    for (auto& rule : rules)
    {
        rule->RegisterASTMatcherCallback(*finder.get());
        rule->RegisterPreProcessorCallbacks(compiler);
    }

    consumers.push_back(finder->newASTConsumer());

    return make_unique<ColobotLintASTConsumer>(std::move(consumers),
                                               std::move(finder),
                                               std::move(rules));
}

///////////////////////////

ColobotLintASTConsumer::ColobotLintASTConsumer(
        std::vector<std::unique_ptr<ASTConsumer>>&& consumers,
        std::unique_ptr<MatchFinder>&& finder,
        std::vector<std::unique_ptr<Rule>>&& rules)
    : MultiplexConsumer(std::move(consumers)),
      m_finder(std::move(finder)),
      m_rules(std::move(rules))
{}
