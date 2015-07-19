#include "ActionFactories.h"

#include "Rules/RulesFactory.h"

#include "llvm/ADT/STLExtras.h"
#include "clang/Lex/Preprocessor.h"

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
    : m_context(context)
{}

bool ColobotLintASTFrontendAction::BeginSourceFileAction(CompilerInstance& ci, StringRef filename)
{
    if (m_context.verbose)
    {
        std::cerr << "Processing " << filename.str() << std::endl;
    }
    return clang::FrontendAction::BeginSourceFileAction(ci, filename);
}

std::unique_ptr<ASTConsumer> ColobotLintASTFrontendAction::CreateASTConsumer(CompilerInstance& compiler,
                                                                             StringRef file)
{
    std::vector<std::unique_ptr<ASTConsumer>> consumers;

    auto finder = make_unique<MatchFinder>();

    auto rules = CreateASTRules(m_context);
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
        std::vector<std::unique_ptr<ASTRule>>&& rules)
    : MultiplexConsumer(std::move(consumers)),
      m_finder(std::move(finder)),
      m_rules(std::move(rules))
{}


///////////////////////////


ColobotLintTokenFrontendActionFactory::ColobotLintTokenFrontendActionFactory(Context& context)
    : m_context(context)
{}

FrontendAction* ColobotLintTokenFrontendActionFactory::create()
{
    return new ColobotLintTokenFrontendAction(m_context);
}

///////////////////////////

ColobotLintTokenFrontendAction::ColobotLintTokenFrontendAction(Context& context)
    : m_context(context)
{
    m_rules = CreateTokenRules(m_context);
}

bool ColobotLintTokenFrontendAction::BeginSourceFileAction(CompilerInstance& ci, StringRef filename)
{
    if (m_context.verbose)
    {
        std::cerr << "Processing " << filename.str() << std::endl;
    }
    return clang::PreprocessorFrontendAction::BeginSourceFileAction(ci, filename);
}

void ColobotLintTokenFrontendAction::ExecuteAction()
{
    Preprocessor& pp = getCompilerInstance().getPreprocessor();
    Token token;
    pp.EnterMainSourceFile();
    do
    {
        pp.Lex(token);
        for (const auto& consumer : m_rules)
        {
            consumer->HandleToken(pp, token);
        }
    }
    while (token.isNot(tok::eof));
}
