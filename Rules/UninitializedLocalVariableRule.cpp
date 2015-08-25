#include "Rules/UninitializedLocalVariableRule.h"

#include "Common/Context.h"
#include "Common/OutputPrinter.h"
#include "Common/SourceLocationHelper.h"
#include "Common/UninitializedPodVariableHelper.h"

#include <clang/AST/Decl.h>

#include <boost/format.hpp>

using namespace clang;
using namespace clang::ast_matchers;


UninitializedLocalVariableRule::UninitializedLocalVariableRule(Context& context)
    : ASTCallbackRule(context)
{}

void UninitializedLocalVariableRule::RegisterASTMatcherCallback(MatchFinder& finder)
{
    finder.addMatcher(varDecl().bind("varDecl"), this);
}

void UninitializedLocalVariableRule::run(const MatchFinder::MatchResult& result)
{
    const VarDecl* variableDeclaration = result.Nodes.getNodeAs<VarDecl>("varDecl");
    if (variableDeclaration == nullptr)
        return;

    SourceManager& sourceManager = result.Context->getSourceManager();

    SourceLocation location = variableDeclaration->getLocation();
    if (! m_context.sourceLocationHelper.IsLocationOfInterest(GetName(), location, sourceManager))
        return;

    if (! variableDeclaration->hasLocalStorage() ||   // skip global/static variables
        ParmVarDecl::classof(variableDeclaration) ||  // ignore function parameters
        variableDeclaration->isImplicit())            // ignore implicit (compiler-generated) variables
    {
        return;
    }

    const DeclContext* declarationContext = variableDeclaration->getDeclContext();
    if (declarationContext->isFunctionOrMethod())
    {
        const FunctionDecl* functionDeclaration = static_cast<const FunctionDecl*>(declarationContext);
        std::string fullyQualifiedName = functionDeclaration->getQualifiedNameAsString();
        // skip old style functions to avoid a flood of errors
        if (m_context.reportedOldStyleFunctions.count(fullyQualifiedName) > 0)
            return;
    }

    if (IsUninitializedPodVariable(variableDeclaration, result.Context))
    {
        m_context.outputPrinter->PrintRuleViolation(
                "uninitialized local variable",
                Severity::Error,
                boost::str(boost::format("Local variable '%s' is uninitialized")
                    % variableDeclaration->getName().str()),
                location,
                sourceManager);
    }
}
