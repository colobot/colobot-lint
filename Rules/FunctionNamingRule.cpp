#include "Rules/FunctionNamingRule.h"

#include "Common/Context.h"
#include "Common/OutputPrinter.h"
#include "Common/RegexHelper.h"
#include "Common/SourceLocationHelper.h"

#include <clang/AST/Decl.h>

#include <boost/format.hpp>

using namespace clang;
using namespace clang::ast_matchers;

namespace clang
{
namespace ast_matchers
{

AST_MATCHER(FunctionDecl, isMethod)
{
    return CXXMethodDecl::classof(&Node);
}

AST_MATCHER(CXXMethodDecl, isConstructor)
{
    return CXXConstructorDecl::classof(&Node);
}

AST_MATCHER(FunctionDecl, isOverloadedOperator)
{
    return Node.isOverloadedOperator();
}

AST_MATCHER(CXXMethodDecl, hasNonSimpleIdentifierName)
{
    // functions with names that are not simple identifiers are for example
    // destructors and conversion operators
    // besides the fact that we should ignore such functions anyway
    // we have to check against this to avoid hitting runtime assert inside getName()
    return !Node.getDeclName().isIdentifier();
}

AST_MATCHER(CXXMethodDecl, isOverriddenVirtualMethod)
{
    // overridden virtual methods in derived classes are not interesting to us
    // we should report violations only once in base class
    return Node.isVirtual() && Node.size_overridden_methods() > 0;
}

AST_MATCHER(CXXMethodDecl, isIteratorAccessMethod)
{
    // iterator access functions for range-based for loop are allowed
    auto name = Node.getName();
    return name == "begin" || name == "end";
}

} // namespace ast_matchers
} // namespace clang

FunctionNamingRule::FunctionNamingRule(Context& context)
    : ASTCallbackRule(context),
      m_functionOrMethodNamePattern(UPPER_CAMEL_CASE_PATTERN)
{}

void FunctionNamingRule::RegisterASTMatcherCallback(MatchFinder& finder)
{
    finder.addMatcher(methodDecl(unless(anyOf(isImplicit(),
                                              isConstructor(),
                                              isOverloadedOperator(),
                                              hasNonSimpleIdentifierName(),
                                              isOverriddenVirtualMethod(),
                                              isIteratorAccessMethod())))
                          .bind("methodDecl"),
                      this);

    finder.addMatcher(functionDecl(unless(anyOf(isImplicit(),
                                                isOverloadedOperator(),
                                                isMethod())))
                          .bind("functionDecl"),
                      this);
}

void FunctionNamingRule::run(const MatchFinder::MatchResult& result)
{
    SourceManager& sourceManager = result.Context->getSourceManager();

    const auto* methodDeclaration = result.Nodes.getNodeAs<CXXMethodDecl>("methodDecl");
    if (methodDeclaration != nullptr)
        return HandleDeclaration("Method", methodDeclaration, sourceManager);

    const auto* functionDeclaration = result.Nodes.getNodeAs<FunctionDecl>("functionDecl");
    if (functionDeclaration != nullptr)
        return HandleDeclaration("Function", functionDeclaration, sourceManager);
}

void FunctionNamingRule::HandleDeclaration(const char* type,
                                           const FunctionDecl* declaration,
                                           SourceManager& sourceManager)
{
    SourceLocation location = declaration->getLocation();
    if (! m_context.sourceLocationHelper.IsLocationOfInterest(GetName(), location, sourceManager))
        return;

    std::string fullyQualifiedName = declaration->getQualifiedNameAsString();
    if (m_reportedFunctionNames.count(fullyQualifiedName) > 0)
        return; // already reported


    auto name = declaration->getName();
    if (! boost::regex_match(name.begin(), name.end(), m_functionOrMethodNamePattern))
    {
        m_context.outputPrinter->PrintRuleViolation(
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
