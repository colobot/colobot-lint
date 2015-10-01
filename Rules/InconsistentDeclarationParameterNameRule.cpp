#include "Rules/InconsistentDeclarationParameterNameRule.h"

#include "Common/Context.h"
#include "Common/OutputPrinter.h"
#include "Common/SourceLocationHelper.h"

#include <clang/AST/Decl.h>

#include <boost/format.hpp>

using namespace clang;
using namespace clang::ast_matchers;


InconsistentDeclarationParameterNameRule::InconsistentDeclarationParameterNameRule(Context& context)
    : Rule(context)
{}

void InconsistentDeclarationParameterNameRule::RegisterASTMatcherCallback(MatchFinder& finder)
{
    finder.addMatcher(
        functionDecl(unless(anyOf(isExpansionInSystemHeader(),
                                  isImplicit())))
            .bind("functionDecl"),
        this);
}

void InconsistentDeclarationParameterNameRule::run(const MatchFinder::MatchResult& result)
{
    const FunctionDecl* functionDeclaration = result.Nodes.getNodeAs<FunctionDecl>("functionDecl");
    if (functionDeclaration == nullptr)
        return;

    SourceManager& sourceManager = result.Context->getSourceManager();

    SourceLocation location = functionDeclaration->getLocation();
    if (! m_context.sourceLocationHelper.IsLocationOfInterest(GetName(), location, sourceManager))
        return;

    const FunctionDecl* canonicalFunctionDeclaration = functionDeclaration->getCanonicalDecl();
    if (m_visitedDeclarations.count(canonicalFunctionDeclaration) > 0)
        return; // already visited

    m_visitedDeclarations.insert(canonicalFunctionDeclaration);

    if (HasInconsitentDeclarationParameters(functionDeclaration))
    {
        m_context.outputPrinter->PrintRuleViolation(
                "inconsistent declaration parameter name",
                Severity::Style,
                boost::str(boost::format("Function '%s' has other declaration(s) with inconsistently named parameter(s)")
                    % functionDeclaration->getQualifiedNameAsString()),
                location,
                sourceManager);
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

