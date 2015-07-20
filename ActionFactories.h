#pragma once

#include "Handlers/BeginSourceFileHandler.h"

#include "Rules/ASTCallbackRule.h"

#include "clang/Frontend/MultiplexConsumer.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Tooling/Tooling.h"

#include <memory>
#include <vector>

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
};

/////////////////////////////////////////////////////////////////////////////////

// Actual AST consumer - this is the actual class that gets AST from Clang frontend
class ColobotLintASTConsumer : public clang::MultiplexConsumer
{
public:
    ColobotLintASTConsumer(std::vector<std::unique_ptr<ASTConsumer>>&& consumers,
                           std::unique_ptr<clang::ast_matchers::MatchFinder>&& finder,
                           std::vector<std::unique_ptr<ASTCallbackRule>>&& rules);

private:
    std::unique_ptr<clang::ast_matchers::MatchFinder> m_finder;
    std::vector<std::unique_ptr<ASTCallbackRule>> m_rules;
};
