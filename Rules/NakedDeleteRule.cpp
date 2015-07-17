#include "NakedDeleteRule.h"

#include "clang/Tooling/Tooling.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/AST/ExprCXX.h"

using namespace llvm;
using namespace clang;
using namespace clang::tooling;
using namespace clang::ast_matchers;

class NakedDeleteRule::Callback : public MatchFinder::MatchCallback
{
public:
    Callback(OutputPrinter& outputPrinter)
        : m_outputPrinter(outputPrinter)
    {}

    virtual void run(const MatchFinder::MatchResult& result) override
    {
        const CXXDeleteExpr* deleteExpr = result.Nodes.getNodeAs<CXXDeleteExpr>("delete");
        if (deleteExpr == nullptr)
            return;

        SourceLocation location = deleteExpr->getLocStart();
        if (! result.Context->getSourceManager().isInMainFile(location))
            return;

        std::string typeStr = deleteExpr->getDestroyedType().getAsString();

        m_outputPrinter.PrintRuleViolation(
            "naked delete",
            std::string("Naked delete called on type ") + typeStr,
            location,
            result.Context);
    }

private:
    OutputPrinter& m_outputPrinter;
};

NakedDeleteRule::NakedDeleteRule(MatchFinder& finder,
                                 OutputPrinter& printer)
    : Rule(printer),
      m_matcher(deleteExpr().bind("delete")),
      m_callback(make_unique<NakedDeleteRule::Callback>(m_printer))
{
    finder.addMatcher(m_matcher, m_callback.get());
}
