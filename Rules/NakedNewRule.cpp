#include "Rules/NakedNewRule.h"

#include "Common/Context.h"
#include "Common/OutputPrinter.h"
#include "Common/SourceLocationHelper.h"

#include <clang/Tooling/Tooling.h>
#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/AST/ExprCXX.h>

#include <boost/format.hpp>

using namespace clang;
using namespace clang::ast_matchers;

NakedNewRule::NakedNewRule(Context& context)
    : Rule(context)
{}

void NakedNewRule::RegisterASTMatcherCallback(MatchFinder& finder)
{
    finder.addMatcher(newExpr().bind("new"), this);
}

void NakedNewRule::run(const MatchFinder::MatchResult& result)
{
    const CXXNewExpr* newExpr = result.Nodes.getNodeAs<CXXNewExpr>("new");
    if (newExpr == nullptr)
        return;

    SourceManager& sourceManager = result.Context->getSourceManager();

    SourceLocation location = newExpr->getLocStart();
    if (! m_context.sourceLocationHelper.IsLocationOfInterest(GetName(), location, sourceManager))
        return;

    std::string typeStr = newExpr->getAllocatedType().getAsString();

    m_context.outputPrinter->PrintRuleViolation(
        "naked new",
        Severity::Warning,
        boost::str(boost::format("Naked new called with type '%s'") % typeStr),
        location,
        sourceManager);
}
