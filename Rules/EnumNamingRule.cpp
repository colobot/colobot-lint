#include "Rules/EnumNamingRule.h"

#include "Common/Context.h"
#include "Common/OutputPrinter.h"
#include "Common/RegexHelper.h"
#include "Common/SourceLocationHelper.h"

#include <boost/format.hpp>

using namespace clang;
using namespace clang::ast_matchers;

EnumNamingRule::EnumNamingRule(Context& context)
    : Rule(context),
      m_enumNamePattern(UPPER_CAMEL_CASE_PATTERN),
      m_enumConstantPattern(UPPER_CAMEL_CASE_PATTERN)
{}

void EnumNamingRule::RegisterASTMatcherCallback(MatchFinder& finder)
{
    finder.addMatcher(enumDecl().bind("enumDecl"), this);
    finder.addMatcher(enumConstantDecl().bind("enumConstantDecl"), this);
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
    SourceManager& sourceManager = context->getSourceManager();

    SourceLocation location = enumDeclaration->getLocation();
    if (! m_context.sourceLocationHelper.IsLocationOfInterest(GetName(), location, sourceManager))
        return;

    StringRef name = enumDeclaration->getName();
    if (name.empty())
    {
        m_context.outputPrinter->PrintRuleViolation(
            "enum naming",
            Severity::Information,
            "Anonymous enum",
            location,
            sourceManager);
        return;
    }

    if (! enumDeclaration->isScopedUsingClassTag())
    {
        m_context.outputPrinter->PrintRuleViolation(
            "enum naming",
            Severity::Information,
            boost::str(boost::format("Old-style enum '%s'") % name.str()),
            location,
            sourceManager);
        return;
    }

    if (! boost::regex_match(name.begin(), name.end(), m_enumNamePattern))
    {
        m_context.outputPrinter->PrintRuleViolation(
            "enum naming",
            Severity::Style,
            boost::str(boost::format("Enum class '%s' should be named in a style like UpperCamelCase") % name.str()),
            location,
            sourceManager);
    }
}

void EnumNamingRule::HandleEnumConstantDeclaration(const clang::EnumConstantDecl* enumConstantDeclaration, ASTContext* context)
{
    SourceManager& sourceManager = context->getSourceManager();

    SourceLocation location = enumConstantDeclaration->getLocation();
    if (! m_context.sourceLocationHelper.IsLocationOfInterest(GetName(), location, sourceManager))
        return;

    const DeclContext* declarationContext = enumConstantDeclaration->getDeclContext();
    if (declarationContext->getDeclKind() == Decl::Kind::Enum)
    {
        const EnumDecl* enumDeclaration = static_cast<const EnumDecl*>(declarationContext);
        if (enumDeclaration->isScopedUsingClassTag())
        {
            StringRef name = enumConstantDeclaration->getName();

            if (! boost::regex_match(name.begin(), name.end(), m_enumConstantPattern))
            {
                m_context.outputPrinter->PrintRuleViolation(
                    "enum naming",
                    Severity::Style,
                    boost::str(boost::format("Enum class constant '%s' should be named in a style like UpperCamelCase")
                        % name.str()),
                    location,
                    sourceManager);
            }
        }
    }
}
