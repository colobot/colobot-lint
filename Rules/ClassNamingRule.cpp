#include "ClassNamingRule.h"

#include "../Common/Context.h"
#include "../Common/OutputPrinter.h"
#include "../Common/SourceLocationHelper.h"
#include "../Common/RegexConsts.h"

using namespace clang;
using namespace clang::ast_matchers;

ClassNamingRule::ClassNamingRule(Context& context)
    : ASTCallbackRule(context),
      m_matcher(recordDecl().bind("recordDecl")),
      m_classNamePattern(std::string("C") + UPPER_CAMEL_CASE_STRING),
      m_structOrUnionNamePattern(UPPER_CAMEL_CASE_STRING)
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

    SourceLocation location = recordDeclaration->getLocation();
    if (! m_context.sourceLocationHelper.IsLocationOfInterest(location, result.Context->getSourceManager()))
        return;

    if (recordDeclaration->isImplicit() ||
        recordDeclaration->isLambda())
    {
        return;
    }

    std::string name = recordDeclaration->getName().str();
    if (name.empty())
    {
        m_context.printer.PrintRuleViolation(
                "class naming",
                Severity::Information,
                "Anonymous " + GetLowercaseRecordTypeString(recordDeclaration),
                location,
                result.Context->getSourceManager());
        return;
    }

    std::string fullyQualifiedName = recordDeclaration->getQualifiedNameAsString();
    if (m_reportedNames.count(fullyQualifiedName) > 0)
        return;

    if (recordDeclaration->isClass())
    {
        if (! boost::regex_match(name, m_classNamePattern))
        {
            m_context.printer.PrintRuleViolation(
                "class naming",
                Severity::Style,
                GetRecordTypeString(recordDeclaration) + " '" + name + "'" + " should be named in a style like CUpperCamelCase",
                location,
                result.Context->getSourceManager());
            m_reportedNames.insert(fullyQualifiedName);
        }
    }
    else if (recordDeclaration->isStruct() ||
             recordDeclaration->isUnion())
    {
        if (! boost::regex_match(name, m_structOrUnionNamePattern))
        {
            m_context.printer.PrintRuleViolation(
                "class naming",
                Severity::Style,
                GetRecordTypeString(recordDeclaration) + " '" + name + "'" + " should be named in a style like UpperCamelCase",
                location,
                result.Context->getSourceManager());
            m_reportedNames.insert(fullyQualifiedName);
        }
        else if (boost::regex_match(name, m_classNamePattern))
        {
            m_context.printer.PrintRuleViolation(
                "class naming",
                Severity::Style,
                GetRecordTypeString(recordDeclaration) + " '" + name + "'" + " follows class naming style CUpperCamelCase but is not a class",
                location,
                result.Context->getSourceManager());
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

