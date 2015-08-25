#include "Rules/OldStyleNullPointerRule.h"

#include "Common/Context.h"
#include "Common/OutputPrinter.h"
#include "Common/SourceLocationHelper.h"

#include <clang/Tooling/Tooling.h>
#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/AST/ExprCXX.h>

#include <boost/format.hpp>

using namespace clang;
using namespace clang::ast_matchers;

OldStyleNullPointerRule::OldStyleNullPointerRule(Context& context)
    : ASTCallbackRule(context)
{}

void OldStyleNullPointerRule::RegisterASTMatcherCallback(MatchFinder& finder)
{
    finder.addMatcher(implicitCastExpr().bind("implicitCastExpr"), this);
}

void OldStyleNullPointerRule::run(const MatchFinder::MatchResult& result)
{
    const ImplicitCastExpr* implicitCastExpr = result.Nodes.getNodeAs<ImplicitCastExpr>("implicitCastExpr");
    if (implicitCastExpr == nullptr)
        return;

    SourceManager& sourceManager = result.Context->getSourceManager();

    SourceLocation location = implicitCastExpr->getLocStart();
    if (! m_context.sourceLocationHelper.IsLocationOfInterest(GetName(), location, sourceManager))
        return;

    if (implicitCastExpr->getCastKind() != CK_NullToPointer)
        return;

    const Stmt* subExpr = implicitCastExpr->getSubExpr();
    if (IntegerLiteral::classof(subExpr))
    {
        m_context.outputPrinter->PrintRuleViolation(
            "old-style null pointer",
            Severity::Style,
            "Use of old-style zero integer literal as null pointer",
            location,
            sourceManager);
    }
}
