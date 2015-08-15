#include "Rules/ImplicitBoolCastRule.h"

#include "Common/Context.h"
#include "Common/OutputPrinter.h"
#include "Common/SourceLocationHelper.h"

#include <clang/Tooling/Tooling.h>
#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/AST/Expr.h>

#include <boost/format.hpp>

using namespace clang;
using namespace clang::ast_matchers;

ImplicitBoolCastRule::ImplicitBoolCastRule(Context& context)
    : ASTCallbackRule(context),
      m_matcher(implicitCastExpr().bind("implicitCastExpr"))
{}

void ImplicitBoolCastRule::RegisterASTMatcherCallback(MatchFinder& finder)
{
    finder.addMatcher(m_matcher, this);
}

void ImplicitBoolCastRule::run(const MatchFinder::MatchResult& result)
{
    const ImplicitCastExpr* implicitCastExpr = result.Nodes.getNodeAs<ImplicitCastExpr>("implicitCastExpr");
    if (implicitCastExpr == nullptr)
        return;

    SourceManager& sourceManager = result.Context->getSourceManager();

    SourceLocation location = implicitCastExpr->getLocStart();
    if (! m_context.sourceLocationHelper.IsLocationOfInterest(GetName(), location, sourceManager))
        return;

    auto kind = implicitCastExpr->getCastKind();
    if (kind == CK_IntegralToBoolean ||
        kind == CK_FloatingToBoolean ||
        kind == CK_PointerToBoolean ||
        kind == CK_MemberPointerToBoolean)
    {
        std::string subExprTypeStr;
        const Expr* subExpr = implicitCastExpr->getSubExpr();
        if (subExpr != nullptr)
        {
            subExprTypeStr = subExpr->getType().getAsString();
        }

        m_context.printer.PrintRuleViolation(
            "implicit bool cast",
            Severity::Warning,
            boost::str(boost::format("Implicit cast '%s' -> bool") % subExprTypeStr),
            location,
            sourceManager);
    }
    else if (kind == CK_IntegralCast || kind == CK_IntegralToFloating)
    {
        std::string subExprTypeStr;
        const Expr* subExpr = implicitCastExpr->getSubExpr();
        if (subExpr != nullptr)
        {
            subExprTypeStr = subExpr->getType().getAsString();
        }

        std::string castTypeStr = implicitCastExpr->getType().getAsString();

        if (CXXBoolLiteralExpr::classof(subExpr) ||
            subExprTypeStr == "_Bool")
        {
            m_context.printer.PrintRuleViolation(
                "implicit bool cast",
                Severity::Warning,
                boost::str(boost::format("Implicit cast bool -> '%s'") % castTypeStr),
                location,
                sourceManager);
        }
    }
}
