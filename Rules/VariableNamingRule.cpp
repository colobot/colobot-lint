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
      m_variableDeclarationMatcher(varDecl().bind("varDecl")),
      m_fieldDeclarationMatcher(fieldDecl().bind("fieldDecl")),
      m_localVariableNamePattern(CAMEL_CASE),
      m_nonConstGlobalVariableNamePattern(std::string("g_") + CAMEL_CASE),
      m_constGlobalVariableNamePattern(ALL_CAPS),
      m_deprecatedVariableNamePattern(std::string("[bp]") + UPPER_CAMEL_CASE), // deprecated bBool and pPtr
      m_publicFieldNamePattern(CAMEL_CASE),
      m_privateOrProtectedFieldNamePattern(std::string("m_") + CAMEL_CASE),
      m_deprecatedFieldNamePattern(std::string("m_[bp]") + UPPER_CAMEL_CASE) // deprecated m_bBool and m_pPtr
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

void VariableNamingRule::HandleVariableDeclaration(const clang::VarDecl* variableDeclaration, clang::ASTContext* context)
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

void VariableNamingRule::ValidateFieldDeclaration(const clang::StringRef& name,
                                                  clang::AccessSpecifier access,
                                                  const clang::SourceLocation& location,
                                                  clang::ASTContext* context)
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
