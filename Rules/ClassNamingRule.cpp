#include "Rules/ClassNamingRule.h"

#include "Common/Context.h"
#include "Common/OutputPrinter.h"
#include "Common/RegexHelper.h"
#include "Common/SourceLocationHelper.h"
#include "Common/TagTypeNameHelper.h"

#include <boost/format.hpp>

using namespace clang;
using namespace clang::ast_matchers;

namespace clang
{
namespace ast_matchers
{

AST_MATCHER(CXXRecordDecl, isLambda)
{
    return Node.isLambda();
}

} // namespace ast_matchers
} // namespace clang

ClassNamingRule::ClassNamingRule(Context& context)
    : Rule(context),
      m_classNamePattern(std::string("C") + UPPER_CAMEL_CASE_PATTERN),
      m_structOrUnionNamePattern(UPPER_CAMEL_CASE_PATTERN)
{}

void ClassNamingRule::RegisterASTMatcherCallback(MatchFinder& finder)
{
    finder.addMatcher(recordDecl(unless(anyOf(isImplicit(), isLambda()))).bind("recordDecl"), this);
}

void ClassNamingRule::run(const MatchFinder::MatchResult& result)
{
    const RecordDecl* recordDeclaration = result.Nodes.getNodeAs<RecordDecl>("recordDecl");
    if (recordDeclaration == nullptr)
        return;

    SourceManager& sourceManager = result.Context->getSourceManager();

    SourceLocation location = recordDeclaration->getLocation();
    if (! m_context.sourceLocationHelper.IsLocationOfInterest(GetName(), location, sourceManager))
        return;

    StringRef name = recordDeclaration->getName();
    if (name.empty())
    {
        m_context.outputPrinter->PrintRuleViolation(
                "class naming",
                Severity::Information,
                boost::str(boost::format("Anonymous %s") % GetLowercaseTagTypeString(recordDeclaration)),
                location,
                sourceManager);
        return;
    }

    const RecordDecl* canonicalRecordDeclaration = static_cast<const RecordDecl*>(recordDeclaration->getCanonicalDecl());

    if (m_visitedDeclarations.count(canonicalRecordDeclaration) > 0)
        return;

    m_visitedDeclarations.insert(canonicalRecordDeclaration);

    if (recordDeclaration->isClass())
    {
        if (! boost::regex_match(name.begin(), name.end(), m_classNamePattern))
        {
            m_context.outputPrinter->PrintRuleViolation(
                "class naming",
                Severity::Style,
                boost::str(boost::format("%s '%s' should be named in a style like CUpperCamelCase")
                    % GetTagTypeString(recordDeclaration)
                    % name.str()),
                location,
                sourceManager);
        }
    }
    else if (recordDeclaration->isStruct() ||
             recordDeclaration->isUnion())
    {
        if (! boost::regex_match(name.begin(), name.end(), m_structOrUnionNamePattern))
        {
            m_context.outputPrinter->PrintRuleViolation(
                "class naming",
                Severity::Style,
                boost::str(boost::format("%s '%s' should be named in a style like UpperCamelCase")
                    % GetTagTypeString(recordDeclaration)
                    % name.str()),
                location,
                sourceManager);
        }
        else if (boost::regex_match(name.begin(), name.end(), m_classNamePattern))
        {
            m_context.outputPrinter->PrintRuleViolation(
                "class naming",
                Severity::Style,
                boost::str(boost::format("%s '%s' follows class naming style CUpperCamelCase but is not a class")
                    % GetTagTypeString(recordDeclaration)
                    % name.str()),
                location,
                sourceManager);
        }
    }
}
