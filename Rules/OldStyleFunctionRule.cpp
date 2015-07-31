#include "Rules/OldStyleFunctionRule.h"

#include "Common/ClassofCast.h"
#include "Common/Context.h"
#include "Common/OutputPrinter.h"
#include "Common/SourceLocationHelper.h"

#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Decl.h>

#include <boost/format.hpp>

using namespace clang;
using namespace clang::ast_matchers;


class OldStyleDeclarationFinder : public RecursiveASTVisitor<OldStyleDeclarationFinder>
{
public:
    OldStyleDeclarationFinder(ASTContext* context);

    bool VisitStmt(Stmt* statement);

    int GetOldStyleDeclarationsCount() const;
    const std::vector<std::string>& GetFirstFewOldStyleDeclarations() const;

private:
    std::unordered_set<std::string> m_oldStyleDeclarations;
    std::unordered_set<std::string> m_correctStyleDeclarations;
    std::vector<std::string> m_firstFewOldStyleDeclarations;
    ASTContext* m_context;
};

////////////////////////


OldStyleFunctionRule::OldStyleFunctionRule(Context& context)
    : ASTCallbackRule(context),
      m_matcher(functionDecl().bind("functionDecl"))
{}

void OldStyleFunctionRule::RegisterASTMatcherCallback(MatchFinder& finder)
{
    finder.addMatcher(m_matcher, this);
}

void OldStyleFunctionRule::run(const MatchFinder::MatchResult& result)
{
    const FunctionDecl* functionDeclaration = result.Nodes.getNodeAs<FunctionDecl>("functionDecl");
    if (functionDeclaration == nullptr)
        return;

    SourceManager& sourceManager = result.Context->getSourceManager();

    SourceLocation location = functionDeclaration->getLocation();
    if (! m_context.sourceLocationHelper.IsLocationOfInterest(GetName(), location, sourceManager))
        return;

    Stmt* body = functionDeclaration->getBody();
    if (body == nullptr)
        return;

    OldStyleDeclarationFinder finder(result.Context);
    finder.TraverseStmt(body);

    int oldStyleDeclarationCount = finder.GetOldStyleDeclarationsCount();
    if (oldStyleDeclarationCount > 0)
    {
        m_context.printer.PrintRuleViolation(
            "old style function",
            Severity::Warning,
            boost::str(boost::format("Function '%s' has variables declared far from point of use %s")
                % functionDeclaration->getNameAsString()
                % GetShortDeclarationsString(finder.GetFirstFewOldStyleDeclarations(), oldStyleDeclarationCount)),
            location,
            sourceManager);

        m_context.reportedOldStyleFunctions.insert(functionDeclaration->getQualifiedNameAsString());
    }
}

std::string OldStyleFunctionRule::GetShortDeclarationsString(const std::vector<std::string>& declarations, int totalCount)
{
    std::string result;
    result += "(";

    int count = 0;
    for (const auto& declaration : declarations)
    {
        if (count > 0)
            result += ", ";

        result += "'";
        result += declaration;
        result += "'";

        ++count;

        if (count >= 4 && totalCount > count)
        {
            result += "... and ";
            result += std::to_string(totalCount - declarations.size());
            result += " more";
            break;
        }
    }

    result += ")";
    return result;
}

////////////////////////

OldStyleDeclarationFinder::OldStyleDeclarationFinder(ASTContext* context)
    : m_context(context)
{}

bool OldStyleDeclarationFinder::VisitStmt(clang::Stmt* statement)
{
    const DeclRefExpr* declarationRef = classof_cast<const DeclRefExpr>(statement);
    if (declarationRef == nullptr)
        return true;

    const VarDecl* variableDeclaration = classof_cast<const VarDecl>(declarationRef->getDecl());
    if (variableDeclaration == nullptr ||
        ParmVarDecl::classof(variableDeclaration) ||
        ! variableDeclaration->hasLocalStorage())
    {
        return true;
    }

    std::string name = variableDeclaration->getNameAsString();

    if (m_oldStyleDeclarations.count(name) > 0 ||
        m_correctStyleDeclarations.count(name) > 0)
        return true; // already visited

    int declarationLineNumber = m_context->getSourceManager().getPresumedLineNumber(variableDeclaration->getLocation());
    int firstUseLineNumber = m_context->getSourceManager().getPresumedLineNumber(statement->getLocStart());

    if (firstUseLineNumber < declarationLineNumber + 3)
    {
        m_correctStyleDeclarations.insert(name);
    }
    else
    {
        m_oldStyleDeclarations.insert(name);
        if (m_firstFewOldStyleDeclarations.size() < 4)
            m_firstFewOldStyleDeclarations.push_back(name);
    }

    return true;
}

int OldStyleDeclarationFinder::GetOldStyleDeclarationsCount() const
{
    return m_oldStyleDeclarations.size();
}

const std::vector<std::string>& OldStyleDeclarationFinder::GetFirstFewOldStyleDeclarations() const
{
    return m_firstFewOldStyleDeclarations;
}
