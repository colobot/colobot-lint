#include "FunctionNamingRule.h"

#include "../Common/ClassofCast.h"
#include "../Common/Context.h"
#include "../Common/OutputPrinter.h"
#include "../Common/RegexHelper.h"
#include "../Common/SourceLocationHelper.h"

#include <clang/AST/Decl.h>

#include <boost/format.hpp>

using namespace clang;
using namespace clang::ast_matchers;

FunctionNamingRule::FunctionNamingRule(Context& context)
    : ASTCallbackRule(context),
      m_matcher(functionDecl().bind("functionDecl")),
      m_functionOrMethodNamePattern(UPPER_CAMEL_CASE_PATTERN)
{}

void FunctionNamingRule::RegisterASTMatcherCallback(MatchFinder& finder)
{
    finder.addMatcher(m_matcher, this);
}

void FunctionNamingRule::run(const MatchFinder::MatchResult& result)
{
    const FunctionDecl* functionDeclaration = result.Nodes.getNodeAs<FunctionDecl>("functionDecl");
    if (functionDeclaration == nullptr)
        return;

    SourceManager& sourceManager = result.Context->getSourceManager();

    SourceLocation location = functionDeclaration->getLocation();
    if (! m_context.sourceLocationHelper.IsLocationOfInterest(GetName(), location, sourceManager))
        return;

    // skip operator overloads
    if (functionDeclaration->isOverloadedOperator())
        return;

    const CXXMethodDecl* methodDeclaration = classof_cast<const CXXMethodDecl>(functionDeclaration);
    if (methodDeclaration != nullptr)
        return HandleMethodDeclaration(methodDeclaration, location, sourceManager);

    return HandleFunctionDeclaration(functionDeclaration, location, sourceManager);
}

void FunctionNamingRule::HandleFunctionDeclaration(const FunctionDecl* functionDeclaration,
                                                   SourceLocation location,
                                                   SourceManager& sourceManager)
{
    auto name = functionDeclaration->getName();
    std::string fullyQualifiedName = functionDeclaration->getQualifiedNameAsString();
    ValidateName("Function", name, fullyQualifiedName, location, sourceManager);
}

void FunctionNamingRule::HandleMethodDeclaration(const CXXMethodDecl* methodDeclaration,
                                                 SourceLocation location,
                                                 SourceManager& sourceManager)
{
    // skip constructors, destructors and conversion operators (operator Type() methods)
    if (CXXDestructorDecl::classof(methodDeclaration) ||
        CXXConstructorDecl::classof(methodDeclaration) ||
        CXXConversionDecl::classof(methodDeclaration))
    {
        return;
    }

    // don't check overridden virtual methods
    if (methodDeclaration->isVirtual() &&
        methodDeclaration->size_overridden_methods() > 0)
    {
        return;
    }

    auto name = methodDeclaration->getName();

    // iterator access functions are an exception
    if (name == "begin" || name == "end")
        return;

    std::string fullyQualifiedName = methodDeclaration->getQualifiedNameAsString();
    ValidateName("Method", name, fullyQualifiedName, location, sourceManager);
}

void FunctionNamingRule::ValidateName(const char* type,
                                      StringRef name,
                                      const std::string& fullyQualifiedName,
                                      SourceLocation location,
                                      SourceManager& sourceManager)
{
    if (m_reportedFunctionNames.count(fullyQualifiedName) > 0)
        return; // already reported


    if (! boost::regex_match(name.begin(), name.end(), m_functionOrMethodNamePattern))
    {
        m_context.printer.PrintRuleViolation(
                "function naming",
                Severity::Style,
                boost::str(boost::format("%s '%s' should be named in UpperCamelCase style")
                    % type
                    % name.str()),
                location,
                sourceManager);

        m_reportedFunctionNames.insert(fullyQualifiedName);
    }
}
