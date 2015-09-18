#include "Rules/NakedDeleteRule.h"

#include "Common/Context.h"
#include "Common/OutputPrinter.h"
#include "Common/SourceLocationHelper.h"

#include <clang/Tooling/Tooling.h>
#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/AST/ExprCXX.h>

#include <boost/format.hpp>

using namespace clang;
using namespace clang::ast_matchers;

NakedDeleteRule::NakedDeleteRule(Context& context)
    : Rule(context)
{}

void NakedDeleteRule::RegisterASTMatcherCallback(MatchFinder& finder)
{
    finder.addMatcher(deleteExpr().bind("delete"), this);
}

void NakedDeleteRule::run(const MatchFinder::MatchResult& result)
{
    const CXXDeleteExpr* deleteExpr = result.Nodes.getNodeAs<CXXDeleteExpr>("delete");
    if (deleteExpr == nullptr)
        return;

    SourceManager& sourceManager = result.Context->getSourceManager();

    SourceLocation location = deleteExpr->getLocStart();
    if (! m_context.sourceLocationHelper.IsLocationOfInterest(GetName(), location, sourceManager))
        return;

    std::string typeStr = deleteExpr->getDestroyedType().getAsString();

    m_context.outputPrinter->PrintRuleViolation(
        "naked delete",
        Severity::Warning,
        boost::str(boost::format("Naked delete called on type '%s'") % typeStr),
        location,
        sourceManager);
}
