#include "Rules/OldStyleFunctionRule.h"

#include "Common/Context.h"
#include "Common/OutputPrinter.h"
#include "Common/PodHelper.h"
#include "Common/SourceLocationHelper.h"
#include "Common/StringRefHash.h"

#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Decl.h>

#include <llvm/ADT/SmallSet.h>
#include <llvm/ADT/SmallVector.h>

#include <boost/format.hpp>

using namespace clang;
using namespace clang::ast_matchers;

namespace
{

const int MAX_FEW_OLD_STYLE_DECLARATIONS = 4;
using FirstFewOldStyleDeclarationsContainer = llvm::SmallVector<StringRef, MAX_FEW_OLD_STYLE_DECLARATIONS>;

class OldStyleDeclarationFinder : public RecursiveASTVisitor<OldStyleDeclarationFinder>
{
public:
    OldStyleDeclarationFinder(ASTContext* context);

    bool VisitStmt(Stmt* statement);

    int GetOldStyleDeclarationsCount() const;
    const FirstFewOldStyleDeclarationsContainer& GetFirstFewOldStyleDeclarations() const;

private:
    bool IsInteresting(const VarDecl* variableDeclaration);

private:
    llvm::SmallSet<StringRef, 10> m_oldStyleDeclarations;
    llvm::SmallSet<StringRef, 10> m_correctStyleDeclarations;
    FirstFewOldStyleDeclarationsContainer m_firstFewOldStyleDeclarations;
    ASTContext* m_context;
};

std::string GetShortDeclarationsString(const FirstFewOldStyleDeclarationsContainer& declarations, int totalCount)
{
    std::string result;
    result += "(";

    int count = 0;
    for (const auto& declaration : declarations)
    {
        if (count > 0)
            result += ", ";

        result += "'";
        result += declaration.str();
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

} // anonymous namespace


////////////////////////


OldStyleFunctionRule::OldStyleFunctionRule(Context& context)
    : Rule(context)
{}

void OldStyleFunctionRule::RegisterASTMatcherCallback(MatchFinder& finder)
{
    finder.addMatcher(functionDecl().bind("functionDecl"), this);
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
        m_context.outputPrinter->PrintRuleViolation(
            "old style function",
            Severity::Warning,
            boost::str(boost::format("Function '%s' seems to be written in legacy C style: "
                                     "it has uninitialized POD type variables declared far from their point of use %s")
                % functionDeclaration->getNameAsString()
                % GetShortDeclarationsString(finder.GetFirstFewOldStyleDeclarations(), oldStyleDeclarationCount)),
            location,
            sourceManager);

        m_context.reportedOldStyleFunctions.insert(functionDeclaration->getQualifiedNameAsString());
    }
}

////////////////////////

OldStyleDeclarationFinder::OldStyleDeclarationFinder(ASTContext* context)
    : m_context(context)
{}

bool OldStyleDeclarationFinder::IsInteresting(const VarDecl* variableDeclaration)
{
    if (variableDeclaration == nullptr ||
        ! variableDeclaration->hasLocalStorage() ||        // skip global/static variables
        ParmVarDecl::classof(variableDeclaration) ||       // ignore function parameters
        variableDeclaration->isImplicit() ||               // ignore implicit (compiler-generated) variables
        ! IsUninitializedPodVariable(variableDeclaration, m_context)) // we're only interested in uninitialized POD types
    {
        return false;
    }

    StringRef name = variableDeclaration->getName();

    // already visited?
    if (m_oldStyleDeclarations.count(name) > 0 ||
        m_correctStyleDeclarations.count(name) > 0)
    {
        return false;
    }

    return true;
}

bool OldStyleDeclarationFinder::VisitStmt(Stmt* statement)
{
    const DeclRefExpr* declarationRef = dyn_cast_or_null<const DeclRefExpr>(statement);
    if (declarationRef == nullptr)
        return true; // recurse further

    const VarDecl* variableDeclaration = dyn_cast_or_null<const VarDecl>(declarationRef->getDecl());
    if (! IsInteresting(variableDeclaration))
        return true; // recurse further

    int declarationLineNumber = m_context->getSourceManager().getPresumedLineNumber(variableDeclaration->getLocation());
    int firstUseLineNumber = m_context->getSourceManager().getPresumedLineNumber(statement->getLocStart());
    StringRef name = variableDeclaration->getName();

    if (firstUseLineNumber < declarationLineNumber + 3)
    {
        m_correctStyleDeclarations.insert(name);
    }
    else
    {
        m_oldStyleDeclarations.insert(name);
        if (m_firstFewOldStyleDeclarations.size() < MAX_FEW_OLD_STYLE_DECLARATIONS)
            m_firstFewOldStyleDeclarations.push_back(name);
    }

    return true; // recurse further
}

int OldStyleDeclarationFinder::GetOldStyleDeclarationsCount() const
{
    return m_oldStyleDeclarations.size();
}

const FirstFewOldStyleDeclarationsContainer& OldStyleDeclarationFinder::GetFirstFewOldStyleDeclarations() const
{
    return m_firstFewOldStyleDeclarations;
}
