#pragma once

#include "Common/Context.h"
#include "Rules/Rule.h"

#include "clang/Frontend/MultiplexConsumer.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Tooling/Tooling.h"

#include <memory>
#include <vector>

// Frontend action factory - this is required by clang::tooling::ClangTool
class ColobotLintActionFactory : public clang::tooling::FrontendActionFactory
{
public:
    ColobotLintActionFactory(Context &context);

    clang::FrontendAction* create() override;

private:
    Context& m_context;
};

/////////////////////////////////////////////////////////////////////////////////

// Frontend action - this is created by above factory
class ColobotLintFrontendAction : public clang::ASTFrontendAction
{
public:
    ColobotLintFrontendAction(Context &context);

    std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance &compiler,
                                                          clang::StringRef file) override;

    bool BeginSourceFileAction(clang::CompilerInstance& ci, llvm::StringRef filename) override;

private:
    Context &m_context;
};

/////////////////////////////////////////////////////////////////////////////////

// Actual AST consumer - this is the actual class that gets AST from Clang frontend
class ColobotLintASTConsumer : public clang::MultiplexConsumer
{
public:
    ColobotLintASTConsumer(std::vector<std::unique_ptr<ASTConsumer>>&& consumers,
                           std::unique_ptr<clang::ast_matchers::MatchFinder>&& finder,
                           std::vector<std::unique_ptr<Rule>>&& rules);

private:
    std::unique_ptr<clang::ast_matchers::MatchFinder> m_finder;
    std::vector<std::unique_ptr<Rule>> m_rules;
};
