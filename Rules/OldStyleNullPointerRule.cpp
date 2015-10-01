#include "Rules/OldStyleNullPointerRule.h"

#include "Common/Context.h"
#include "Common/OutputPrinter.h"
#include "Common/SourceLocationHelper.h"

#include <clang/Tooling/Tooling.h>
#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/AST/ExprCXX.h>

#include <boost/format.hpp>

using namespace llvm;
using namespace clang;
using namespace clang::ast_matchers;

namespace clang
{
namespace ast_matchers
{

const internal::VariadicDynCastAllOfMatcher<Stmt, GNUNullExpr> gnuNullExpr;

AST_MATCHER(ImplicitCastExpr, isIntegerLiteralToPointerCast)
{
    return Node.getCastKind() == CK_NullToPointer && isa<IntegerLiteral>(Node.getSubExpr());
}

} // namespace ast_matchers
} // namespace clang

OldStyleNullPointerRule::OldStyleNullPointerRule(Context& context)
    : Rule(context)
{}

void OldStyleNullPointerRule::RegisterASTMatcherCallback(MatchFinder& finder)
{
    finder.addMatcher(
        implicitCastExpr(unless(isExpansionInSystemHeader()),
                         isIntegerLiteralToPointerCast())
            .bind("zeroLiteralNullExpr"),
        this);

    finder.addMatcher(expr(has(gnuNullExpr().bind("gnuNullExpr"))).bind("parentExpr"), this);
}

void OldStyleNullPointerRule::run(const MatchFinder::MatchResult& result)
{
    const Expr* zeroLiteralNullExpression = result.Nodes.getNodeAs<Expr>("zeroLiteralNullExpr");
    if (zeroLiteralNullExpression != nullptr)
        return handleZeroLiteralNullExpression(zeroLiteralNullExpression, result.Context->getSourceManager());

    const GNUNullExpr* gnuNullExpression = result.Nodes.getNodeAs<GNUNullExpr>("gnuNullExpr");
    const Expr* parentExpression = result.Nodes.getNodeAs<Expr>("parentExpr");
    if (gnuNullExpression != nullptr)
        return handleGnuNullExpression(gnuNullExpression, parentExpression, result.Context->getSourceManager());
}

void OldStyleNullPointerRule::handleZeroLiteralNullExpression(const Expr* zeroLiteralNullExpression,
                                                              SourceManager& sourceManager)
{
    SourceLocation location = zeroLiteralNullExpression->getLocStart();
    if (m_context.sourceLocationHelper.IsLocationOfInterest(GetName(), location, sourceManager))
    {
        m_context.outputPrinter->PrintRuleViolation(
                "old-style null pointer",
                Severity::Style,
                "Use of old-style zero integer literal as null pointer",
                location,
                sourceManager);
    }
}

void OldStyleNullPointerRule::handleGnuNullExpression(const GNUNullExpr* gnuNullExpression,
                                                      const Expr* parentExpression,
                                                      SourceManager& sourceManager)
{
    SourceLocation location = gnuNullExpression->getLocStart();
    if (m_context.sourceLocationHelper.IsLocationOfInterestAllowingMacros(GetName(), location, sourceManager) &&
        m_context.sourceLocationHelper.IsLocationOfInterest(GetName(), parentExpression->getLocStart(), sourceManager))
    {
        m_context.outputPrinter->PrintRuleViolation(
            "old-style null pointer",
            Severity::Style,
            "Use of NULL macro (GNU __null extension) as null pointer",
            location,
            sourceManager);
    }
}
