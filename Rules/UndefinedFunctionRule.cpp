#include "Rules/UndefinedFunctionRule.h"

#include "Common/Context.h"
#include "Common/FilenameHelper.h"
#include "Common/OutputPrinter.h"
#include "Common/SourceLocationHelper.h"

#include <clang/AST/Decl.h>

#include <boost/format.hpp>

using namespace clang;
using namespace clang::ast_matchers;

namespace clang
{
namespace ast_matchers
{

AST_MATCHER(FunctionDecl, isAnyTemplateKind)
{
    return Node.getTemplatedKind() != FunctionDecl::TK_NonTemplate;
}

AST_MATCHER(FunctionDecl, isDefaulted)
{
    return Node.isDefaulted();
}

} // namespace ast_matchers
} // namespace clang


UndefinedFunctionRule::UndefinedFunctionRule(Context& context)
    : Rule(context)
{}

void UndefinedFunctionRule::RegisterASTMatcherCallback(clang::ast_matchers::MatchFinder& finder)
{
    finder.addMatcher(
        functionDecl(unless(anyOf(isImplicit(),
                                  isDefaulted(),
                                  isDeleted(),
                                  isAnyTemplateKind())),
                     unless(methodDecl(isPure())))
            .bind("functionDecl"),
    this);
}

void UndefinedFunctionRule::run(const clang::ast_matchers::MatchFinder::MatchResult& result)
{
    m_sourceManager = result.SourceManager;

    const FunctionDecl* functionDeclaration = result.Nodes.getNodeAs<FunctionDecl>("functionDecl");
    if (functionDeclaration == nullptr)
        return;

    SourceLocation location = functionDeclaration->getLocation();
    if (! m_context.sourceLocationHelper.IsLocationOfInterest(GetName(), location, *m_sourceManager))
        return;

    std::string fullyQualifiedName = functionDeclaration->getQualifiedNameAsString();
    if (functionDeclaration->isThisDeclarationADefinition())
    {
        m_context.definedFunctions.insert(fullyQualifiedName);
        m_context.undefinedFunctions.erase(fullyQualifiedName);
    }
    else
    {
        if (m_context.definedFunctions.count(fullyQualifiedName) == 0)
        {
            SourceFileInfo info;
            info.fileName = GetCleanFilename(location, *m_sourceManager);
            info.lineNumber = m_sourceManager->getPresumedLineNumber(location);
            m_context.undefinedFunctions.insert(std::make_pair(fullyQualifiedName, info));
        }
    }
}

void UndefinedFunctionRule::onEndOfTranslationUnit()
{
    m_context.outputPrinter->ClearTentativeViolations();

    for (const auto& undefinedFunction : m_context.undefinedFunctions)
    {
        const bool tentative = true;
        m_context.outputPrinter->PrintRuleViolation(
            "undefined function",
            Severity::Information,
            boost::str(boost::format("Function '%s' declared but never defined")
                % undefinedFunction.first),
            undefinedFunction.second.fileName,
            undefinedFunction.second.lineNumber,
            tentative);
    }
}
