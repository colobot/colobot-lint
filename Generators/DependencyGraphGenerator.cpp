#include "Generators/DependencyGraphGenerator.h"

#include "Common/Context.h"
#include "Common/OutputPrinter.h"
#include "Common/SourceLocationHelper.h"

using namespace clang;
using namespace clang::ast_matchers;

DependencyGraphGenerator::DependencyGraphGenerator(Context& context)
    : Generator(context),
      m_recordDeclMatcher(recordDecl().bind("recordDecl")),
      m_memberCallExprMatcher(memberCallExpr(hasAncestor(methodDecl(hasDeclContext(recordDecl().bind("callerRecordDecl"))))).bind("memberCallExpr"))
{}

void DependencyGraphGenerator::RegisterASTMatcherCallback(MatchFinder& finder)
{
    finder.addMatcher(m_recordDeclMatcher, this);
    finder.addMatcher(m_memberCallExprMatcher, this);
}

void DependencyGraphGenerator::run(const MatchFinder::MatchResult& result)
{
    SourceManager& sourceManager = result.Context->getSourceManager();

    const CXXRecordDecl* recordDecl = result.Nodes.getNodeAs<CXXRecordDecl>("recordDecl");
    if (recordDecl != nullptr)
        return HandleRecordDeclaration(recordDecl, sourceManager);

    const CXXMemberCallExpr* memberCallExpr = result.Nodes.getNodeAs<CXXMemberCallExpr>("memberCallExpr");
    const RecordDecl* callerRecordDecl = result.Nodes.getNodeAs<RecordDecl>("callerRecordDecl");
    if (memberCallExpr != nullptr && callerRecordDecl != nullptr)
        return HandleMemberCallExpression(memberCallExpr, callerRecordDecl, sourceManager);
}

void DependencyGraphGenerator::HandleRecordDeclaration(const CXXRecordDecl* recordDecl,
                                                       SourceManager& sourceManager)
{
    SourceLocation location = recordDecl->getLocation();
    if (! m_context.sourceLocationHelper.IsLocationOfInterestIgnoringExclusionZone(location, sourceManager))
        return;

    if (!recordDecl->isThisDeclarationADefinition())
        return;

    std::string recordType = recordDecl->getQualifiedNameAsString();

    LangOptions languageOptions;
    PrintingPolicy printingPolicy(languageOptions);
    printingPolicy.SuppressTagKeyword = true;

    for (const CXXBaseSpecifier& base : recordDecl->bases())
    {
        std::string baseType = base.getType().getAsString(printingPolicy);
        m_context.printer.PrintGraphEdge(recordType, baseType, "[color=\"red\"]");
    }
}

void DependencyGraphGenerator::HandleMemberCallExpression(const CXXMemberCallExpr* memberCallExpr,
                                                          const RecordDecl* callerRecordDecl,
                                                          SourceManager& sourceManager)
{
    SourceLocation location = memberCallExpr->getLocStart();
    if (! m_context.sourceLocationHelper.IsLocationOfInterestIgnoringExclusionZone(location, sourceManager))
        return;

    const CXXRecordDecl* calleeRecordDecl = memberCallExpr->getRecordDecl();
    if (calleeRecordDecl == nullptr ||
        !m_context.sourceLocationHelper.IsLocationInProjectSourceFile(calleeRecordDecl->getLocation(), sourceManager))
    {
        return;
    }

    std::string callerRecordType = callerRecordDecl->getQualifiedNameAsString();
    std::string calleeRecordType = calleeRecordDecl->getQualifiedNameAsString();
    if (callerRecordType != calleeRecordType)
    {
        m_context.printer.PrintGraphEdge(callerRecordType, calleeRecordType);
    }
}
