#include "UninitializedLocalVariableRule.h"

#include "../Common/Context.h"
#include "../Common/OutputPrinter.h"
#include "../Common/SourceLocationHelper.h"

#include <clang/AST/Decl.h>

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

    SourceLocation location = variableDeclaration->getLocation();
    if (! m_context.sourceLocationHelper.IsLocationOfInterest(location, result.Context->getSourceManager()))
        return;

    // Ignore implicit (compiler-generated) variables
    if (variableDeclaration->isImplicit())
        return;

    if (! variableDeclaration->hasLocalStorage())
        return;

    QualType type = variableDeclaration->getType();
    if (! type.isPODType(*result.Context))
        return;

    if (!variableDeclaration->hasInit() || (type->isRecordType() && HasImplicitInitialization(variableDeclaration)))
    {
        m_context.printer.PrintRuleViolation(
                "uninitialized local variable",
                Severity::Error,
                std::string("Local variable '") + variableDeclaration->getName().str() + "' is uninitialized",
                location,
                result.Context->getSourceManager());
    }
}

bool UninitializedLocalVariableRule::HasImplicitInitialization(const VarDecl* variableDeclaration)
{
    const Expr* initExpr = variableDeclaration->getInit();
    if (initExpr == nullptr)
        return false;

    if (! CXXConstructExpr::classof(initExpr))
        return false;

    const CXXConstructExpr* constructExpr = static_cast<const CXXConstructExpr*>(initExpr);
    if (constructExpr->getConstructor() == nullptr)
        return false;

    return constructExpr->getConstructor()->isImplicit();
}
