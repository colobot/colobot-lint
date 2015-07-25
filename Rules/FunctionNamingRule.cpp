#include "FunctionNamingRule.h"

#include "../Common/Context.h"
#include "../Common/OutputPrinter.h"
#include "../Common/SourceLocationHelper.h"
#include "../Common/RegexConsts.h"

#include <clang/AST/Decl.h>

using namespace clang;
using namespace clang::ast_matchers;

FunctionNamingRule::FunctionNamingRule(Context& context)
    : ASTCallbackRule(context),
      m_matcher(functionDecl().bind("functionDecl")),
      m_functionOrMethodNamePattern(UPPER_CAMEL_CASE_STRING)
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

    SourceLocation location = functionDeclaration->getLocation();
    if (! m_context.sourceLocationHelper.IsLocationOfInterest(location, result.Context->getSourceManager()))
        return;

    // skip operator overloads
    if (functionDeclaration->isOverloadedOperator())
        return;

    if (CXXMethodDecl::classof(functionDeclaration))
        return HandleMethodDeclaration(static_cast<const CXXMethodDecl*>(functionDeclaration), result.Context);

    return HandleFunctionDeclaration(functionDeclaration, location, result.Context);
}

void FunctionNamingRule::HandleFunctionDeclaration(const FunctionDecl* functionDeclaration,
                                                   const SourceLocation& location,
                                                   ASTContext* context)
{
    auto name = functionDeclaration->getName();
    std::string fullyQualifiedName = functionDeclaration->getQualifiedNameAsString();
    ValidateName("Function", name.str(), fullyQualifiedName, location, context);
}

void FunctionNamingRule::HandleMethodDeclaration(const CXXMethodDecl* methodDeclaration, ASTContext* context)
{
    SourceLocation location = methodDeclaration->getLocation();
    if (! m_context.sourceLocationHelper.IsLocationOfInterest(location, context->getSourceManager()))
        return;

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
    ValidateName("Method", name.str(), fullyQualifiedName, location, context);
}

void FunctionNamingRule::ValidateName(const char* type, const std::string& name,
                                      const std::string& fullyQualifiedName,
                                      const SourceLocation& location, ASTContext* context)
{
    if (m_reportedFunctionNames.count(fullyQualifiedName) > 0)
        return; // already reported


    if (! boost::regex_match(name, m_functionOrMethodNamePattern))
    {
        m_context.printer.PrintRuleViolation(
                "function naming",
                Severity::Style,
                std::string(type) + " '" + name + "'" + " should be named in UpperCamelCase style",
                location,
                context->getSourceManager());

        m_reportedFunctionNames.insert(fullyQualifiedName);
    }
}
