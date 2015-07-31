#include "Rules/ClassNamingRule.h"

#include "Common/Context.h"
#include "Common/OutputPrinter.h"
#include "Common/RegexHelper.h"
#include "Common/SourceLocationHelper.h"

#include <boost/format.hpp>

using namespace clang;
using namespace clang::ast_matchers;

ClassNamingRule::ClassNamingRule(Context& context)
    : ASTCallbackRule(context),
      m_matcher(recordDecl().bind("recordDecl")),
      m_classNamePattern(std::string("C") + UPPER_CAMEL_CASE_PATTERN),
      m_structOrUnionNamePattern(UPPER_CAMEL_CASE_PATTERN)
{}

void ClassNamingRule::RegisterASTMatcherCallback(MatchFinder& finder)
{
    finder.addMatcher(m_matcher, this);
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

    if (recordDeclaration->isImplicit() ||
        recordDeclaration->isLambda())
    {
        return;
    }

    StringRef name = recordDeclaration->getName();
    if (name.empty())
    {
        m_context.printer.PrintRuleViolation(
                "class naming",
                Severity::Information,
                boost::str(boost::format("Anonymous %s") % GetLowercaseRecordTypeString(recordDeclaration)),
                location,
                sourceManager);
        return;
    }

    std::string fullyQualifiedName = recordDeclaration->getQualifiedNameAsString();
    if (m_reportedNames.count(fullyQualifiedName) > 0)
        return;

    if (recordDeclaration->isClass())
    {
        if (! boost::regex_match(name.begin(), name.end(), m_classNamePattern))
        {
            m_context.printer.PrintRuleViolation(
                "class naming",
                Severity::Style,
                boost::str(boost::format("%s '%s' should be named in a style like CUpperCamelCase")
                    % GetRecordTypeString(recordDeclaration)
                    % name.str()),
                location,
                sourceManager);
            m_reportedNames.insert(fullyQualifiedName);
        }
    }
    else if (recordDeclaration->isStruct() ||
             recordDeclaration->isUnion())
    {
        if (! boost::regex_match(name.begin(), name.end(), m_structOrUnionNamePattern))
        {
            m_context.printer.PrintRuleViolation(
                "class naming",
                Severity::Style,
                boost::str(boost::format("%s '%s' should be named in a style like UpperCamelCase")
                    % GetRecordTypeString(recordDeclaration)
                    % name.str()),
                location,
                sourceManager);
            m_reportedNames.insert(fullyQualifiedName);
        }
        else if (boost::regex_match(name.begin(), name.end(), m_classNamePattern))
        {
            m_context.printer.PrintRuleViolation(
                "class naming",
                Severity::Style,
                boost::str(boost::format("%s '%s' follows class naming style CUpperCamelCase but is not a class")
                    % GetRecordTypeString(recordDeclaration)
                    % name.str()),
                location,
                sourceManager);
            m_reportedNames.insert(fullyQualifiedName);
        }
    }
}

std::string ClassNamingRule::GetRecordTypeString(const clang::RecordDecl* recordDeclaration)
{
    if (recordDeclaration->isClass())
        return "Class";
    else if (recordDeclaration->isUnion())
        return "Union";
    else if (recordDeclaration->isStruct())
        return "Struct";
    return "";
}

std::string ClassNamingRule::GetLowercaseRecordTypeString(const clang::RecordDecl* recordDeclaration)
{
    if (recordDeclaration->isClass())
        return "class";
    else if (recordDeclaration->isUnion())
        return "union";
    else if (recordDeclaration->isStruct())
        return "struct";
    return "";
}

