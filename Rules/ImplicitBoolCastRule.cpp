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
    : Rule(context)
{}

void ImplicitBoolCastRule::RegisterASTMatcherCallback(MatchFinder& finder)
{
    finder.addMatcher(implicitCastExpr(unless(isExpansionInSystemHeader())).bind("implicitCastExpr"), this);
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

    if (IsImplicitCastToBool(implicitCastExpr))
    {
        std::string subExprTypeStr = implicitCastExpr->getSubExpr()->getType().getAsString();

        m_context.outputPrinter->PrintRuleViolation(
            "implicit bool cast",
            Severity::Warning,
            boost::str(boost::format("Implicit cast '%s' -> bool") % subExprTypeStr),
            location,
            sourceManager);
    }
    else if (IsImplicitCastFromBool(implicitCastExpr, result.Context))
    {
        std::string castTypeStr = implicitCastExpr->getType().getAsString();

        m_context.outputPrinter->PrintRuleViolation(
                "implicit bool cast",
                Severity::Warning,
                boost::str(boost::format("Implicit cast bool -> '%s'") % castTypeStr),
                location,
                sourceManager);
    }
}

bool ImplicitBoolCastRule::IsImplicitCastToBool(const ImplicitCastExpr* implicitCastExpr)
{
    auto kind = implicitCastExpr->getCastKind();

    return kind == CK_IntegralToBoolean ||
           kind == CK_FloatingToBoolean ||
           kind == CK_PointerToBoolean ||
           kind == CK_MemberPointerToBoolean;
}

bool ImplicitBoolCastRule::IsImplicitCastFromBool(const ImplicitCastExpr* implicitCastExpr,
                                                  ASTContext* astContext,
                                                  bool checkBoolComparison)
{
    if (implicitCastExpr == nullptr)
        return false;

    auto kind = implicitCastExpr->getCastKind();

    // we're only interested in casts to integer and floating types
    // (well, actually, other types don't make much sense here)
    if (kind != CK_IntegralCast &&
        kind != CK_IntegralToFloating)
    {
        return false;
    }

    // comparison of bools with == and != operators is allowed
    if (checkBoolComparison &&
        kind == CK_IntegralCast &&
        IsComparisonOfBools(implicitCastExpr, astContext))
    {
        return false;
    }

    const Expr* subExpr = implicitCastExpr->getSubExpr();
    if (subExpr == nullptr)
        return false;

    return CXXBoolLiteralExpr::classof(subExpr) ||
           subExpr->getType().getAsString() == "_Bool";
}

bool ImplicitBoolCastRule::IsComparisonOfBools(const ImplicitCastExpr* implicitCastExpr, ASTContext* astContext)
{
    auto parents = astContext->getParents(*implicitCastExpr);
    if (parents.size() == 0)
        return false;

    const BinaryOperator* binaryOperator = dyn_cast_or_null<const BinaryOperator>(parents.front().get<Stmt>());
    if (binaryOperator == nullptr)
        return false;

    auto opcode = binaryOperator->getOpcode();
    if (opcode != BO_EQ &&
        opcode != BO_NE)
    {
        return false;
    }

    const ImplicitCastExpr* lhsImplicitCast = dyn_cast_or_null<const ImplicitCastExpr>(binaryOperator->getLHS());
    const ImplicitCastExpr* rhsImplicitCast = dyn_cast_or_null<const ImplicitCastExpr>(binaryOperator->getRHS());
    return IsImplicitCastFromBool(lhsImplicitCast, astContext, false) &&
           IsImplicitCastFromBool(rhsImplicitCast, astContext, false);
}
