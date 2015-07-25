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
    if (name.empty()) // anonymous structs or unions
        return;

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
                std::string("Class '") + name + "'" + " should be named in a style like CUpperCamelCase",
                location,
                result.Context->getSourceManager());
            m_reportedNames.insert(fullyQualifiedName);
        }
    }
    else if (recordDeclaration->isStruct() ||
             recordDeclaration->isUnion())
    {
        std::string which = recordDeclaration->isStruct() ? "Struct" : "Union";

        if (! boost::regex_match(name, m_structOrUnionNamePattern))
        {
            m_context.printer.PrintRuleViolation(
                "class naming",
                Severity::Style,
                which + " '" + name + "'" + " should be named in a style like UpperCamelCase",
                location,
                result.Context->getSourceManager());
            m_reportedNames.insert(fullyQualifiedName);
        }
        else if (boost::regex_match(name, m_classNamePattern))
        {
            m_context.printer.PrintRuleViolation(
                "class naming",
                Severity::Style,
                which + " '" + name + "'" + " follows class naming style CUpperCamelCase but is not a class",
                location,
                result.Context->getSourceManager());
            m_reportedNames.insert(fullyQualifiedName);
        }
    }
}
