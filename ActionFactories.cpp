#include "ActionFactories.h"

#include "Rules/RulesFactory.h"

#include "llvm/ADT/STLExtras.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/AST/Decl.h"
#include "clang/AST/RecursiveASTVisitor.h"

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
      m_beginSourceFileHandler(context)
{}

bool ColobotLintASTFrontendAction::BeginSourceFileAction(CompilerInstance& ci, StringRef filename)
{
    m_beginSourceFileHandler.BeginSourceFileAction(ci, filename);
    return clang::FrontendAction::BeginSourceFileAction(ci, filename);
}

std::unique_ptr<ASTConsumer> ColobotLintASTFrontendAction::CreateASTConsumer(CompilerInstance& compiler,
                                                                             StringRef file)
{
    std::vector<std::unique_ptr<ASTConsumer>> consumers;

    auto finder = make_unique<MatchFinder>();

    auto astCallbackRules = CreateASTRules(m_context);
    for (auto& rule : astCallbackRules)
    {
        rule->RegisterASTMatcherCallback(*finder.get());
        rule->RegisterPreProcessorCallbacks(compiler);
    }

    consumers.push_back(finder->newASTConsumer());

    auto directAstConsumerRules = CreateDirectASTConsumerRules(m_context);
    for (auto& rule : directAstConsumerRules)
    {
        consumers.push_back(std::move(rule));
    }

    return make_unique<ColobotLintASTConsumer>(std::move(consumers),
                                               std::move(finder),
                                               std::move(astCallbackRules));
}

///////////////////////////

ColobotLintASTConsumer::ColobotLintASTConsumer(
        std::vector<std::unique_ptr<ASTConsumer>>&& consumers,
        std::unique_ptr<MatchFinder>&& finder,
        std::vector<std::unique_ptr<ASTCallbackRule>>&& rules)
    : MultiplexConsumer(std::move(consumers)),
      m_finder(std::move(finder)),
      m_rules(std::move(rules))
{}
