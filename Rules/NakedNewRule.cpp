#include "NakedNewRule.h"

#include "../Common/SourceLocationHelper.h"

#include "clang/Tooling/Tooling.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/AST/ExprCXX.h"

using namespace clang;
using namespace clang::ast_matchers;

NakedNewRule::NakedNewRule(Context& context)
    : ASTCallbackRule(context),
      m_matcher(newExpr().bind("new"))
{}

void NakedNewRule::RegisterASTMatcherCallback(MatchFinder& finder)
{
    finder.addMatcher(m_matcher, this);
}

void NakedNewRule::run(const MatchFinder::MatchResult& result)
{
    const CXXNewExpr* newExpr = result.Nodes.getNodeAs<CXXNewExpr>("new");
    if (newExpr == nullptr)
        return;

    SourceLocation location = newExpr->getLocStart();
    if (! m_context.sourceLocationHelper.IsLocationOfInterest(location, result.Context->getSourceManager()))
        return;

    std::string typeStr = newExpr->getAllocatedType().getAsString();

    m_context.printer.PrintRuleViolation(
        "naked new",
        Severity::Warning,
        std::string("Naked new called with type '") + typeStr + "'",
        location,
        result.Context->getSourceManager());
}
