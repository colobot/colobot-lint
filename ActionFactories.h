#pragma once

#include "Handlers/BeginSourceFileHandler.h"
#include "Handlers/ExclusionZoneCommentHandler.h"

#include <clang/Frontend/MultiplexConsumer.h>
#include <clang/Tooling/Tooling.h>

#include <memory>
#include <vector>

class Rule;
class Generator;

namespace clang
{
namespace ast_matchers
{
class MatchFinder;
} // namespace ast_matchers
} // namespace clang

// Frontend action factory for AST checkers - this is required by clang::tooling::ClangTool interface
class ColobotLintASTFrontendActionFactory : public clang::tooling::FrontendActionFactory
{
public:
    ColobotLintASTFrontendActionFactory(Context &context);

    clang::FrontendAction* create() override;

private:
    Context& m_context;
};

/////////////////////////////////////////////////////////////////////////////////

// Frontend action for AST checkers - this is created by above factory
class ColobotLintASTFrontendAction : public clang::ASTFrontendAction
{
public:
    ColobotLintASTFrontendAction(Context &context);

    std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance &compiler,
                                                          clang::StringRef file) override;

    bool BeginSourceFileAction(clang::CompilerInstance& ci, llvm::StringRef filename) override;

private:
    Context &m_context;
    BeginSourceFileHandler m_beginSourceFileHandler;
    ExclusionZoneCommentHandler m_exclusionZoneCommentHandler;
};

/////////////////////////////////////////////////////////////////////////////////

// Actual AST consumer - this is the actual class that gets AST from Clang frontend
class ColobotLintASTConsumer : public clang::MultiplexConsumer
{
public:
    ColobotLintASTConsumer(std::vector<std::unique_ptr<ASTConsumer>>&& consumers,
                           std::unique_ptr<clang::ast_matchers::MatchFinder>&& finder,
                           std::vector<std::unique_ptr<Rule>>&& rules,
                           std::unique_ptr<Generator>&& generator);
    ~ColobotLintASTConsumer();

private:
    std::unique_ptr<clang::ast_matchers::MatchFinder> m_finder;
    std::vector<std::unique_ptr<Rule>> m_rules;
    std::unique_ptr<Generator> m_generator;
};
