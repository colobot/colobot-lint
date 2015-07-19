#pragma once

#include "Common/BeginSourceFileHandler.h"

#include "Rules/ASTRule.h"
#include "Rules/TokenRule.h"

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
                           std::vector<std::unique_ptr<ASTRule>>&& rules);

private:
    std::unique_ptr<clang::ast_matchers::MatchFinder> m_finder;
    std::vector<std::unique_ptr<ASTRule>> m_rules;
};

/////////////////////////////////////////////////////////////////////////////////

// Frontend action factory for AST checkers - this is required by clang::tooling::ClangTool interface
class ColobotLintTokenFrontendActionFactory : public clang::tooling::FrontendActionFactory
{
public:
    ColobotLintTokenFrontendActionFactory(Context &context);

    clang::FrontendAction* create() override;

private:
    Context& m_context;
};

/////////////////////////////////////////////////////////////////////////////////

// Frontend action for AST checkers - this is created by above factory
class ColobotLintTokenFrontendAction : public clang::PreprocessorFrontendAction
{
public:
    ColobotLintTokenFrontendAction(Context &context);

    bool BeginSourceFileAction(clang::CompilerInstance& ci, llvm::StringRef filename) override;
    void ExecuteAction() override;

private:
    Context &m_context;
    BeginSourceFileHandler m_beginSourceFileHandler;
    std::vector<std::unique_ptr<TokenRule>> m_rules;
};
