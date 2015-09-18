#include "Rules/VariableNamingRule.h"

#include "Common/Context.h"
#include "Common/OutputPrinter.h"
#include "Common/RegexHelper.h"
#include "Common/SourceLocationHelper.h"

#include <clang/AST/Decl.h>

#include <boost/format.hpp>

using namespace clang;
using namespace clang::ast_matchers;



VariableNamingRule::VariableNamingRule(Context& context)
    : Rule(context),
      m_localVariableNamePattern(LOWER_CAMEL_CASE_PATTERN),
      m_nonConstGlobalVariableNamePattern(std::string("g_") + LOWER_CAMEL_CASE_PATTERN),
      m_constGlobalVariableNamePattern(ALL_CAPS_UNDERSCORE_PATTERN),
      m_deprecatedVariableNamePattern("[bp][[:upper:]].*"), // deprecated bBool and pPtr
      m_publicFieldNamePattern(LOWER_CAMEL_CASE_PATTERN),
      m_privateOrProtectedFieldNamePattern(std::string("m_") + LOWER_CAMEL_CASE_PATTERN),
      m_deprecatedFieldNamePattern("m_[bp][[:upper:]].*") // deprecated m_bBool and m_pPtr
{}

void VariableNamingRule::RegisterASTMatcherCallback(MatchFinder& finder)
{
    finder.addMatcher(varDecl().bind("varDecl"), this);
    finder.addMatcher(fieldDecl().bind("fieldDecl"), this);
}

void VariableNamingRule::run(const MatchFinder::MatchResult& result)
{
    const VarDecl* variableDeclaration = result.Nodes.getNodeAs<VarDecl>("varDecl");
    if (variableDeclaration != nullptr)
        return HandleVariableDeclaration(variableDeclaration, result.Context->getSourceManager());

    const FieldDecl* fieldDeclaration = result.Nodes.getNodeAs<FieldDecl>("fieldDecl");
    if (fieldDeclaration != nullptr)
        return HandleFieldDeclaration(fieldDeclaration, result.Context->getSourceManager());
}

void VariableNamingRule::HandleVariableDeclaration(const VarDecl* variableDeclaration,
                                                   SourceManager& sourceManager)
{
    SourceLocation location = variableDeclaration->getLocation();
    if (! m_context.sourceLocationHelper.IsLocationOfInterest(GetName(), location, sourceManager))
        return;

    // Ignore implicit (compiler-generated) variables
    if (variableDeclaration->isImplicit())
        return;

    auto name = variableDeclaration->getName();

    // Unnamed parameters are fine
    if (name.empty())
        return;

    // Static class members follow same rules as regular class members
    if (variableDeclaration->isStaticDataMember())
    {
        ValidateFieldDeclaration(name, variableDeclaration->getAccess(), location, sourceManager);
    }
    // Local, non-static variables in functions
    else if (variableDeclaration->hasLocalStorage())
    {
        if (! boost::regex_match(name.begin(), name.end(), m_localVariableNamePattern))
        {
              m_context.outputPrinter->PrintRuleViolation(
                "variable naming",
                Severity::Style,
                boost::str(boost::format("Local variable '%s' should be named in camelCase style")
                    % name.str()),
                location,
                sourceManager);
        }
        else if (boost::regex_match(name.begin(), name.end(), m_deprecatedVariableNamePattern))
        {
            m_context.outputPrinter->PrintRuleViolation(
                "variable naming",
                Severity::Style,
                boost::str(boost::format("Local variable '%s' is named in a style that is deprecated")
                    % name.str()),
                location,
                sourceManager);
        }
    }
    // Global variables and constants
    else if (variableDeclaration->hasGlobalStorage())
    {
        if (variableDeclaration->getType().isConstQualified())
        {
            if (! boost::regex_match(name.begin(), name.end(), m_constGlobalVariableNamePattern))
            {
                m_context.outputPrinter->PrintRuleViolation(
                    "variable naming",
                    Severity::Style,
                    boost::str(boost::format("Const global variable '%s' should be named in ALL_CAPS style")
                        % name.str()),
                    location,
                    sourceManager);
            }
        }
        else
        {
            if (! boost::regex_match(name.begin(), name.end(), m_nonConstGlobalVariableNamePattern))
            {
                m_context.outputPrinter->PrintRuleViolation(
                    "variable naming",
                    Severity::Style,
                    boost::str(boost::format("Non-const global variable '%s' should be named in g_camelCase style")
                        % name.str()),
                    location,
                    sourceManager);
            }
        }
    }
}

void VariableNamingRule::HandleFieldDeclaration(const FieldDecl* fieldDeclaration,
                                                SourceManager& sourceManager)
{
    SourceLocation location = fieldDeclaration->getLocation();
    if (! m_context.sourceLocationHelper.IsLocationOfInterest(GetName(), location, sourceManager))
        return;

    auto name = fieldDeclaration->getName();

    ValidateFieldDeclaration(name, fieldDeclaration->getAccess(), location, sourceManager);
}

void VariableNamingRule::ValidateFieldDeclaration(StringRef name,
                                                  AccessSpecifier access,
                                                  SourceLocation location,
                                                  SourceManager& sourceManager)
{
    if (access == AS_public)
    {
        if (! boost::regex_match(name.begin(), name.end(), m_publicFieldNamePattern))
        {
            m_context.outputPrinter->PrintRuleViolation(
                "variable naming",
                Severity::Style,
                boost::str(boost::format("Public field '%s' should be named in camelCase style")
                    % name.str()),
                location,
                sourceManager);
        }
    }
    else if (access == AS_protected || access == AS_private)
    {
        if (! boost::regex_match(name.begin(), name.end(), m_privateOrProtectedFieldNamePattern))
        {
            std::string which = (access == AS_protected) ? "Protected" : "Private";
            m_context.outputPrinter->PrintRuleViolation(
                "variable naming",
                Severity::Style,
                boost::str(boost::format("%s field '%s' should be named in m_camelCase style")
                    % which
                    % name.str()),
                location,
                sourceManager);
        }
        else if (boost::regex_match(name.begin(), name.end(), m_deprecatedFieldNamePattern))
        {
            std::string which = (access == AS_protected) ? "Protected" : "Private";
            m_context.outputPrinter->PrintRuleViolation(
                "variable naming",
                Severity::Style,
                boost::str(boost::format("%s field '%s' is named in a style that is deprecated")
                    % which
                    % name.str()),
                location,
                sourceManager);
        }
    }
}
