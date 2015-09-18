#include "ActionFactories.h"

#include "Generators/GeneratorsFactory.h"

#include "Rules/Rule.h"
#include "Rules/RulesFactory.h"

#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Lex/Preprocessor.h>

using namespace llvm;
using namespace clang;
using namespace clang::ast_matchers;


ColobotLintASTFrontendActionFactory::ColobotLintASTFrontendActionFactory(Context& context)
    : m_context(context)
{}

FrontendAction* ColobotLintASTFrontendActionFactory::create()
{
    return new ColobotLintASTFrontendAction(m_context);
}

///////////////////////////

ColobotLintASTFrontendAction::ColobotLintASTFrontendAction(Context& context)
    : m_context(context),
      m_beginSourceFileHandler(context),
      m_exclusionZoneCommentHandler(context)
{}

bool ColobotLintASTFrontendAction::BeginSourceFileAction(CompilerInstance& ci, StringRef filename)
{
    return m_beginSourceFileHandler.BeginSourceFileAction(ci, filename);
}

std::unique_ptr<ASTConsumer> ColobotLintASTFrontendAction::CreateASTConsumer(CompilerInstance& compiler,
                                                                             StringRef /*file*/)
{
    std::vector<std::unique_ptr<ASTConsumer>> consumers;

    auto finder = make_unique<MatchFinder>();

    std::vector<std::unique_ptr<Rule>> rules;

    std::unique_ptr<Generator> generator = CreateGenerator(m_context);
    if (generator != nullptr)
    {
        generator->RegisterASTMatcherCallback(*finder.get());
    }
    else
    {
        compiler.getPreprocessor().addCommentHandler(&m_exclusionZoneCommentHandler);

        rules = CreateRules(m_context);
        for (auto& rule : rules)
        {
            rule->RegisterASTMatcherCallback(*finder.get());
            rule->RegisterPreProcessorCallbacks(compiler);
        }
    }

    consumers.push_back(finder->newASTConsumer());

    return make_unique<ColobotLintASTConsumer>(std::move(consumers),
                                               std::move(finder),
                                               std::move(rules),
                                               std::move(generator));
}

///////////////////////////

ColobotLintASTConsumer::ColobotLintASTConsumer(
        std::vector<std::unique_ptr<ASTConsumer>>&& consumers,
        std::unique_ptr<MatchFinder>&& finder,
        std::vector<std::unique_ptr<Rule>>&& rules,
        std::unique_ptr<Generator>&& generator)
    : MultiplexConsumer(std::move(consumers)),
      m_finder(std::move(finder)),
      m_rules(std::move(rules)),
      m_generator(std::move(generator))
{}

ColobotLintASTConsumer::~ColobotLintASTConsumer()
{}

