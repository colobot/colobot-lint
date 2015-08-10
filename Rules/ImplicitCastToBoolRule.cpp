#include "Rules/ImplicitCastToBoolRule.h"

#include "Common/Context.h"
#include "Common/OutputPrinter.h"
#include "Common/SourceLocationHelper.h"

#include <clang/Tooling/Tooling.h>
#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/AST/Expr.h>

#include <boost/format.hpp>

using namespace clang;
using namespace clang::ast_matchers;

ImplicitCastToBoolRule::ImplicitCastToBoolRule(Context& context)
    : ASTCallbackRule(context),
      m_matcher(implicitCastExpr().bind("implicitCastExpr"))
{}

void ImplicitCastToBoolRule::RegisterASTMatcherCallback(MatchFinder& finder)
{
    finder.addMatcher(m_matcher, this);
}

void ImplicitCastToBoolRule::run(const MatchFinder::MatchResult& result)
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
        std::string typeStr;
        const Expr* subExpr = implicitCastExpr->getSubExpr();
        if (subExpr != nullptr)
        {
            typeStr = subExpr->getType().getAsString();
        }

        m_context.printer.PrintRuleViolation(
            "implicit cast to bool",
            Severity::Warning,
            boost::str(boost::format("Implicit cast '%s' -> bool") % typeStr),
            location,
            sourceManager);
    }
}
