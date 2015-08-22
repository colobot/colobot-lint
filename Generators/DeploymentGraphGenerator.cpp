#include "Generators/DeploymentGraphGenerator.h"

#include "Common/Context.h"
#include "Common/OutputPrinter.h"
#include "Common/SourceLocationHelper.h"


using namespace clang;
using namespace clang::ast_matchers;

DeploymentGraphGenerator::DeploymentGraphGenerator(Context& context)
    : Generator(context),
      m_recordDeclMatcher(recordDecl().bind("recordDecl")),
      m_uniquePtrFieldDeclMatcher(
          fieldDecl(
            hasType(qualType(hasDescendant(
                qualType(
                    hasDeclaration(recordDecl(hasName("unique_ptr")))
                ).bind("uniquePtrType")
            )))
        ).bind("fieldDecl"))
{}

void DeploymentGraphGenerator::RegisterASTMatcherCallback(MatchFinder& finder)
{
    finder.addMatcher(m_recordDeclMatcher, this);
    finder.addMatcher(m_uniquePtrFieldDeclMatcher, this);
}

void DeploymentGraphGenerator::run(const MatchFinder::MatchResult& result)
{
    const CXXRecordDecl* recordDecl = result.Nodes.getNodeAs<CXXRecordDecl>("recordDecl");
    if (recordDecl != nullptr)
        return HandleRecordDeclaration(recordDecl, *result.SourceManager);

    const FieldDecl* fieldDecl = result.Nodes.getNodeAs<FieldDecl>("fieldDecl");
    const QualType* uniquePtrType = result.Nodes.getNodeAs<QualType>("uniquePtrType");
    if (fieldDecl != nullptr && uniquePtrType != nullptr)
        return HandleUniquePtrFieldDeclaration(fieldDecl, uniquePtrType, result.Context);
}

void DeploymentGraphGenerator::HandleRecordDeclaration(const CXXRecordDecl* recordDecl,
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
        m_context.outputPrinter->PrintGraphEdge(recordType, baseType, "[color=\"red\"]");
    }
}

void DeploymentGraphGenerator::HandleUniquePtrFieldDeclaration(const FieldDecl* fieldDecl,
                                                               const QualType* uniquePtrType,
                                                               ASTContext* astContext)
{
    if (! m_context.sourceLocationHelper.IsLocationOfInterestIgnoringExclusionZone(
              fieldDecl->getLocation(), astContext->getSourceManager()))
    {
        return;
    }

    if (!fieldDecl->getDeclContext()->isRecord())
        return;

    const RecordDecl* recordDecl = static_cast<const RecordDecl*>(fieldDecl->getDeclContext());

    const RecordType* uniquePtrRecordType =
        uniquePtrType->getDesugaredType(*astContext)->getAs<const RecordType>();

    if (uniquePtrRecordType == nullptr)
        return;

    const ClassTemplateSpecializationDecl* uniquePtrSpecialization = dyn_cast_or_null<const ClassTemplateSpecializationDecl>(
        uniquePtrRecordType->getDecl());

    if (uniquePtrSpecialization == nullptr)
        return;

    QualType actualPtrType = uniquePtrSpecialization->getTemplateArgs().get(0).getAsType();
    if (!actualPtrType->isRecordType())
        return;

    std::string recordTypeString = recordDecl->getQualifiedNameAsString();

    LangOptions languageOptions;
    PrintingPolicy printingPolicy(languageOptions);
    printingPolicy.SuppressTagKeyword = true;
    std::string actualPtrTypeString = actualPtrType.getAsString(printingPolicy);

    m_context.outputPrinter->PrintGraphEdge(recordTypeString, actualPtrTypeString);
}
