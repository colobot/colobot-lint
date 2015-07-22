#include "VariableNamingRule.h"

#include "../Common/Context.h"
#include "../Common/OutputPrinter.h"
#include "../Common/SourceLocationHelper.h"

#include <clang/AST/Decl.h>

using namespace clang;
using namespace ast_matchers;

namespace
{
const char* CAMEL_CASE = "[a-z]+([A-Z0-9]+[a-z0-9]+)*";
const char* UPPER_CAMEL_CASE = "[A-Z][a-z]+([A-Z0-9]+[a-z0-9]+)*";
const char* ALL_CAPS = "[A-Z]+(_[A-Z0-9]+)*";
} // anonymous namespace

VariableNamingRule::VariableNamingRule(Context& context)
    : ASTCallbackRule(context),
      m_matcher(varDecl().bind("varDecl")),
      m_localVariableNamePattern(CAMEL_CASE),
      m_nonConstGlobalVariableNamePattern(std::string("g_") + CAMEL_CASE),
      m_constGlobalVariableNamePattern(ALL_CAPS),
      m_deprecatedVariableNamePattern(std::string("[bp]") + UPPER_CAMEL_CASE) // deprecated bBool and pPtr
{}

void VariableNamingRule::RegisterASTMatcherCallback(MatchFinder& finder)
{
    finder.addMatcher(m_matcher, this);
}

void VariableNamingRule::run(const MatchFinder::MatchResult& result)
{
    const VarDecl* variableDeclaration = result.Nodes.getNodeAs<VarDecl>("varDecl");
    if (variableDeclaration == nullptr)
        return;

    SourceLocation location = variableDeclaration->getLocation();
    if (! m_context.sourceLocationHelper.IsLocationOfInterest(location, result.Context->getSourceManager()))
        return;

    auto name = variableDeclaration->getName();

    // local, non-static variables in functions
    if (variableDeclaration->hasLocalStorage())
    {
        if (! boost::regex_match(name.str(), m_localVariableNamePattern))
        {
              m_context.printer.PrintRuleViolation(
                "variable naming",
                Severity::Style,
                std::string("Local variable '") + name.str() + "'" + " should be named in camelCase style",
                location,
                result.Context->getSourceManager());
        }
        else if (boost::regex_match(name.str(), m_deprecatedVariableNamePattern))
        {
            m_context.printer.PrintRuleViolation(
                "variable naming",
                Severity::Style,
                std::string("Local variable '") + name.str() + "'" + " is named in a style that is deprecated",
                location,
                result.Context->getSourceManager());
        }
    }
    // global variables and constants
    else if (variableDeclaration->hasGlobalStorage())
    {
        if (variableDeclaration->getType().isConstQualified())
        {
            if (! boost::regex_match(name.str(), m_constGlobalVariableNamePattern))
            {
                m_context.printer.PrintRuleViolation(
                    "variable naming",
                    Severity::Style,
                    std::string("Const global variable '") + name.str() + "'" + " should be named in ALL_CAPS style",
                    location,
                    result.Context->getSourceManager());
            }
        }
        else
        {
            if (! boost::regex_match(name.str(), m_nonConstGlobalVariableNamePattern))
            {
                m_context.printer.PrintRuleViolation(
                    "variable naming",
                    Severity::Style,
                    std::string("Non-const global variable '") + name.str() + "'" + " should be named in g_camelCase style",
                    location,
                    result.Context->getSourceManager());
            }
        }
    }
}
