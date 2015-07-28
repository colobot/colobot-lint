#include "InconsistentDeclarationParameterNameRule.h"

#include "../Common/Context.h"
#include "../Common/OutputPrinter.h"
#include "../Common/SourceLocationHelper.h"

#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Decl.h>

using namespace clang;
using namespace clang::ast_matchers;


InconsistentDeclarationParameterNameRule::InconsistentDeclarationParameterNameRule(Context& context)
    : ASTCallbackRule(context),
      m_matcher(functionDecl().bind("functionDecl"))
{}

void InconsistentDeclarationParameterNameRule::RegisterASTMatcherCallback(MatchFinder& finder)
{
    finder.addMatcher(m_matcher, this);
}

void InconsistentDeclarationParameterNameRule::run(const MatchFinder::MatchResult& result)
{
    const FunctionDecl* functionDeclaration = result.Nodes.getNodeAs<FunctionDecl>("functionDecl");
    if (functionDeclaration == nullptr)
        return;

    SourceLocation location = functionDeclaration->getLocation();
    if (! m_context.sourceLocationHelper.IsLocationOfInterest(location, result.Context->getSourceManager()))
        return;

    if (functionDeclaration->isImplicit())
        return;

    std::string fullyQualifiedName = functionDeclaration->getQualifiedNameAsString();
    if (m_reportedFunctions.count(fullyQualifiedName) > 0)
        return; // already visited

    if (HasInconsitentDeclarationParameters(functionDeclaration))
    {
        m_context.printer.PrintRuleViolation(
                "inconsistent declaration parameter name",
                Severity::Style,
                std::string("Function '") + functionDeclaration->getNameAsString() +
                    "' has other declaration(s) with inconsistently named parameter(s)",
                location,
                result.Context->getSourceManager());

        m_reportedFunctions.insert(fullyQualifiedName);
    }
}

bool InconsistentDeclarationParameterNameRule::HasInconsitentDeclarationParameters(const FunctionDecl* functionDeclaration)
{
    SourceLocation location = functionDeclaration->getLocation();

    for (const auto& otherDeclaration : functionDeclaration->redecls())
    {
        if (otherDeclaration->getLocation() == location) // skip current one
            continue;

        auto myParamIt = functionDeclaration->param_begin();
        auto otherParamIt = otherDeclaration->param_begin();
        while (myParamIt != functionDeclaration->param_end() &&
               otherParamIt != otherDeclaration->param_end())
        {
            auto myParamName = (*myParamIt)->getName();
            auto otherParamName = (*otherParamIt)->getName();

            if (!myParamName.empty() && !otherParamName.empty() && myParamName != otherParamName)
            {
                return true;
            }
            ++myParamIt;
            ++otherParamIt;
        }
    }

    return false;
}

