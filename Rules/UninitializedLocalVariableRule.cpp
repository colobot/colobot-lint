#include "UninitializedLocalVariableRule.h"

#include "../Common/ClassofCast.h"
#include "../Common/Context.h"
#include "../Common/OutputPrinter.h"
#include "../Common/SourceLocationHelper.h"

#include <clang/AST/Decl.h>

#include <boost/format.hpp>

using namespace clang;
using namespace clang::ast_matchers;


UninitializedLocalVariableRule::UninitializedLocalVariableRule(Context& context)
    : ASTCallbackRule(context),
      m_matcher(varDecl().bind("varDecl"))
{}

void UninitializedLocalVariableRule::RegisterASTMatcherCallback(MatchFinder& finder)
{
    finder.addMatcher(m_matcher, this);
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

    if (! variableDeclaration->hasLocalStorage() ||
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

    QualType type = variableDeclaration->getType();
    if (! type.isPODType(*result.Context))
        return;

    if (!variableDeclaration->hasInit() || (type->isRecordType() && HasImplicitInitialization(variableDeclaration)))
    {
        m_context.printer.PrintRuleViolation(
                "uninitialized local variable",
                Severity::Error,
                boost::str(boost::format("Local variable '%s' is uninitialized")
                    % variableDeclaration->getName().str()),
                location,
                sourceManager);
    }
}

bool UninitializedLocalVariableRule::HasImplicitInitialization(const VarDecl* variableDeclaration)
{
    const CXXConstructExpr* constructExpr = classof_cast<const CXXConstructExpr>(variableDeclaration->getInit());
    if (constructExpr == nullptr ||
        constructExpr->getConstructor() == nullptr)
    {
        return false;
    }

    return constructExpr->getConstructor()->isImplicit();
}
