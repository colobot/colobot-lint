#include "NakedDeleteRule.h"

#include "clang/Tooling/Tooling.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/AST/ExprCXX.h"

using namespace clang;
using namespace clang::ast_matchers;

NakedDeleteRule::NakedDeleteRule(MatchFinder& finder,
                                 OutputPrinter& printer)
    : Rule(printer),
      m_matcher(deleteExpr().bind("delete"))
{
    finder.addMatcher(m_matcher, this);
}

void NakedDeleteRule::run(const MatchFinder::MatchResult& result)
{
    const CXXDeleteExpr* deleteExpr = result.Nodes.getNodeAs<CXXDeleteExpr>("delete");
    if (deleteExpr == nullptr)
        return;

    SourceLocation location = deleteExpr->getLocStart();
    if (! result.Context->getSourceManager().isInMainFile(location))
        return;

    std::string typeStr = deleteExpr->getDestroyedType().getAsString();

    m_printer.PrintRuleViolation(
        "naked delete",
        Severity::Warning,
        std::string("Naked delete called on type '") + typeStr + "'",
        location,
        result.Context->getSourceManager());
}
