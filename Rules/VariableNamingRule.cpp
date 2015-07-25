#include "VariableNamingRule.h"

#include "../Common/Context.h"
#include "../Common/OutputPrinter.h"
#include "../Common/SourceLocationHelper.h"
#include "../Common/RegexConsts.h"

#include <clang/AST/Decl.h>

using namespace clang;
using namespace clang::ast_matchers;



VariableNamingRule::VariableNamingRule(Context& context)
    : ASTCallbackRule(context),
      m_variableDeclarationMatcher(varDecl().bind("varDecl")),
      m_fieldDeclarationMatcher(fieldDecl().bind("fieldDecl")),
      m_localVariableNamePattern(LOWER_CAMEL_CASE_STRING),
      m_nonConstGlobalVariableNamePattern(std::string("g_") + LOWER_CAMEL_CASE_STRING),
      m_constGlobalVariableNamePattern(ALL_CAPS_UNDERSCORE_STRING),
      m_deprecatedVariableNamePattern("[bp][[:upper:]].*"), // deprecated bBool and pPtr
      m_publicFieldNamePattern(LOWER_CAMEL_CASE_STRING),
      m_privateOrProtectedFieldNamePattern(std::string("m_") + LOWER_CAMEL_CASE_STRING),
      m_deprecatedFieldNamePattern("m_[bp][[:upper:]].*") // deprecated m_bBool and m_pPtr
{}

void VariableNamingRule::RegisterASTMatcherCallback(MatchFinder& finder)
{
    finder.addMatcher(m_variableDeclarationMatcher, this);
    finder.addMatcher(m_fieldDeclarationMatcher, this);
}

void VariableNamingRule::run(const MatchFinder::MatchResult& result)
{
    const VarDecl* variableDeclaration = result.Nodes.getNodeAs<VarDecl>("varDecl");
    if (variableDeclaration != nullptr)
        return HandleVariableDeclaration(variableDeclaration, result.Context);

    const FieldDecl* fieldDeclaration = result.Nodes.getNodeAs<FieldDecl>("fieldDecl");
    if (fieldDeclaration != nullptr)
        return HandleFieldDeclaration(fieldDeclaration, result.Context);
}

void VariableNamingRule::HandleVariableDeclaration(const VarDecl* variableDeclaration, ASTContext* context)
{
    SourceLocation location = variableDeclaration->getLocation();
    if (! m_context.sourceLocationHelper.IsLocationOfInterest(location, context->getSourceManager()))
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
        ValidateFieldDeclaration(name, variableDeclaration->getAccess(), location, context);
    }
    // Local, non-static variables in functions
    else if (variableDeclaration->hasLocalStorage())
    {
        if (! boost::regex_match(name.str(), m_localVariableNamePattern))
        {
              m_context.printer.PrintRuleViolation(
                "variable naming",
                Severity::Style,
                std::string("Local variable '") + name.str() + "'" + " should be named in camelCase style",
                location,
                context->getSourceManager());
        }
        else if (boost::regex_match(name.str(), m_deprecatedVariableNamePattern))
        {
            m_context.printer.PrintRuleViolation(
                "variable naming",
                Severity::Style,
                std::string("Local variable '") + name.str() + "'" + " is named in a style that is deprecated",
                location,
                context->getSourceManager());
        }
    }
    // Global variables and constants
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
                    context->getSourceManager());
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
                    context->getSourceManager());
            }
        }
    }
}

void VariableNamingRule::HandleFieldDeclaration(const FieldDecl* fieldDeclaration, ASTContext* context)
{
    SourceLocation location = fieldDeclaration->getLocation();
    if (! m_context.sourceLocationHelper.IsLocationOfInterest(location, context->getSourceManager()))
        return;

    auto name = fieldDeclaration->getName();

    ValidateFieldDeclaration(name, fieldDeclaration->getAccess(), location, context);
}

void VariableNamingRule::ValidateFieldDeclaration(const StringRef& name,
                                                  AccessSpecifier access,
                                                  const SourceLocation& location,
                                                  ASTContext* context)
{
    if (access == AS_public)
    {
        if (! boost::regex_match(name.str(), m_publicFieldNamePattern))
        {
            m_context.printer.PrintRuleViolation(
                "variable naming",
                Severity::Style,
                std::string("Public field '") + name.str() + "'" + " should be named in camelCase style",
                location,
                context->getSourceManager());
        }
    }
    else if (access == AS_protected || access == AS_private)
    {
        if (! boost::regex_match(name.str(), m_privateOrProtectedFieldNamePattern))
        {
            std::string which = (access == AS_protected) ? "Protected" : "Private";
            m_context.printer.PrintRuleViolation(
                "variable naming",
                Severity::Style,
                which +  " field '" + name.str() + "'" + " should be named in m_camelCase style",
                location,
                context->getSourceManager());
        }
        else if (boost::regex_match(name.str(), m_deprecatedFieldNamePattern))
        {
            std::string which = (access == AS_protected) ? "Protected" : "Private";
            m_context.printer.PrintRuleViolation(
                "variable naming",
                Severity::Style,
                which +  " field '" + name.str() + "'" + " is named in a style that is deprecated",
                location,
                context->getSourceManager());
        }
    }
}
