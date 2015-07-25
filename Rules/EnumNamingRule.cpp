#include "EnumNamingRule.h"

#include "../Common/Context.h"
#include "../Common/OutputPrinter.h"
#include "../Common/SourceLocationHelper.h"
#include "../Common/RegexConsts.h"

using namespace clang;
using namespace clang::ast_matchers;

EnumNamingRule::EnumNamingRule(Context& context)
    : ASTCallbackRule(context),
      m_enumDeclarationMatcher(enumDecl().bind("enumDecl")),
      m_enumConstantDeclarationMatcher(enumConstantDecl().bind("enumConstantDecl")),
      m_enumNamePattern(UPPER_CAMEL_CASE_STRING),
      m_enumConstantPattern(UPPER_CAMEL_CASE_STRING)
{}

void EnumNamingRule::RegisterASTMatcherCallback(MatchFinder& finder)
{
    finder.addMatcher(m_enumDeclarationMatcher, this);
    finder.addMatcher(m_enumConstantDeclarationMatcher, this);
}

void EnumNamingRule::run(const MatchFinder::MatchResult& result)
{
    const EnumDecl* enumDeclaration = result.Nodes.getNodeAs<EnumDecl>("enumDecl");
    if (enumDeclaration != nullptr)
        return HandleEnumDeclaration(enumDeclaration, result.Context);

    const EnumConstantDecl* enumConstantDeclaration = result.Nodes.getNodeAs<EnumConstantDecl>("enumConstantDecl");
    if (enumConstantDeclaration != nullptr)
        return HandleEnumConstantDeclaration(enumConstantDeclaration, result.Context);
}

void EnumNamingRule::HandleEnumDeclaration(const clang::EnumDecl* enumDeclaration, ASTContext* context)
{
    SourceLocation location = enumDeclaration->getLocation();
    if (! m_context.sourceLocationHelper.IsLocationOfInterest(location, context->getSourceManager()))
        return;

    std::string name = enumDeclaration->getName().str();
    if (name.empty()) // enums can be anonymous
        return;

    if (! enumDeclaration->isScopedUsingClassTag())
    {
        m_context.printer.PrintRuleViolation(
            "enum naming",
            Severity::Information,
            std::string("Old-style enum '") + name + "'",
            location,
            context->getSourceManager());
        return;
    }

    if (! boost::regex_match(name, m_enumNamePattern))
    {
        m_context.printer.PrintRuleViolation(
            "enum naming",
            Severity::Style,
            std::string("Enum class '") + name + "'" + " should be named in a style like UpperCamelCase",
            location,
            context->getSourceManager());
    }
}

void EnumNamingRule::HandleEnumConstantDeclaration(const clang::EnumConstantDecl* enumConstantDeclaration, ASTContext* context)
{
    SourceLocation location = enumConstantDeclaration->getLocation();
    if (! m_context.sourceLocationHelper.IsLocationOfInterest(location, context->getSourceManager()))
        return;

    const DeclContext* declarationContext = enumConstantDeclaration->getDeclContext();
    if (declarationContext->getDeclKind() == Decl::Kind::Enum)
    {
        const EnumDecl* enumDeclaration = static_cast<const EnumDecl*>(declarationContext);
        if (enumDeclaration->isScopedUsingClassTag())
        {
            std::string name = enumConstantDeclaration->getName().str();

            if (! boost::regex_match(name, m_enumConstantPattern))
            {
                m_context.printer.PrintRuleViolation(
                    "enum naming",
                    Severity::Style,
                    std::string("Enum class constant '") + name + "'" + " should be named in a style like UpperCamelCase",
                    location,
                    context->getSourceManager());
            }
        }
    }
}
