#include "Rules/PossibleForwardDeclarationRule.h"

#include "Common/Context.h"
#include "Common/OutputPrinter.h"
#include "Common/SourceLocationHelper.h"
#include "Common/TagTypeMatchers.h"
#include "Common/TagTypeNameHelper.h"

#include <clang/AST/Decl.h>

#include <boost/format.hpp>

#include <algorithm>
#include <vector>

using namespace clang;
using namespace clang::ast_matchers;

namespace
{

QualType RecursivelyDesugarType(const QualType& type)
{
    QualType desugaredType = type;

    while (true)
    {
        const ElaboratedType* elaboratedType = desugaredType->getAs<ElaboratedType>();
        if (elaboratedType != nullptr)
        {
            desugaredType = elaboratedType->desugar();
            continue;
        }

        if (desugaredType->isArrayType())
        {
            desugaredType = desugaredType->getAsArrayTypeUnsafe()->getElementType();
            continue;
        }

        const PointerType* pointerType = desugaredType->getAs<PointerType>();
        if (pointerType != nullptr)
        {
            desugaredType = pointerType->getPointeeType();
            continue;
        }

        const ReferenceType* referenceType = desugaredType->getAs<ReferenceType>();
        if (referenceType != nullptr)
        {
            desugaredType = referenceType->getPointeeType();
            continue;
        }

        break;
    }

    return desugaredType;
}

QualType RecursivelyDesugarPtrOrRefType(const QualType& type)
{
    const PointerType* pointerType = type->getAs<PointerType>();
    if (pointerType != nullptr)
    {
        return RecursivelyDesugarType(pointerType->getPointeeType());
    }

    const ReferenceType* referenceType = type->getAs<ReferenceType>();
    if (referenceType != nullptr)
    {
        return RecursivelyDesugarType(referenceType->getPointeeType());
    }

    return QualType();
}

QualType RecursivelyDesugarTypedefType(const QualType& type)
{
    const TypedefType* typedefType = type->getAs<TypedefType>();
    if (typedefType != nullptr)
    {
        return RecursivelyDesugarType(typedefType->desugar());
    }

    return QualType();
}

} // anonymous namespace

namespace clang
{
namespace ast_matchers
{

AST_MATCHER_P(QualType, recursivelyDesugaredType, internal::Matcher<QualType>, InnerMatcher)
{
    if (Node.isNull())
        return false;

    QualType desugaredType = RecursivelyDesugarType(Node);
    return InnerMatcher.matches(desugaredType, Finder, Builder);
}

AST_MATCHER_P(QualType, recursivelyDesugaredPtrOrRefType, internal::Matcher<QualType>, InnerMatcher)
{
    if (Node.isNull())
        return false;

    QualType desugaredType = RecursivelyDesugarPtrOrRefType(Node);
    return InnerMatcher.matches(desugaredType, Finder, Builder);
}

AST_MATCHER_P(QualType, recursivelyDesugaredTypedefType, internal::Matcher<QualType>, InnerMatcher)
{
    if (Node.isNull())
        return false;

    QualType desugaredType = RecursivelyDesugarTypedefType(Node);
    return InnerMatcher.matches(desugaredType, Finder, Builder);
}

} // namespace ast_matchers
} // namespace clang


internal::Matcher<QualType> CreateActualTagTypeMatcher()
{
    return qualType(hasDeclaration(tagDecl(unless(isExpansionInSystemHeader())).bind("tagDecl")));
}

internal::Matcher<QualType> CreateTemplateTagTypeMatcher()
{
    return templateSpecializationType(
        hasAnyTemplateArgument(
            refersToType(
                recursivelyDesugaredType(CreateActualTagTypeMatcher()))));
}

internal::Matcher<QualType> CreateTagTypeMatcher()
{
    return anyOf(qualType(recursivelyDesugaredTypedefType(CreateActualTagTypeMatcher())),
                 qualType(recursivelyDesugaredType(CreateTemplateTagTypeMatcher())),
                 qualType(recursivelyDesugaredPtrOrRefType(CreateActualTagTypeMatcher())).bind("ptrOrRefType"),
                 qualType(recursivelyDesugaredType(CreateActualTagTypeMatcher())));
}

PossibleForwardDeclarationRule::PossibleForwardDeclarationRule(Context& context)
    : Rule(context)
{}

void PossibleForwardDeclarationRule::RegisterASTMatcherCallback(MatchFinder& finder)
{
    finder.addMatcher(recordDecl().bind("recordDecl"), this);
    finder.addMatcher(declRefExpr().bind("declRefExpr"), this);
    finder.addMatcher(
        decl(anyOf(valueDecl(hasType(CreateTagTypeMatcher())),
                   functionDecl(returns(CreateTagTypeMatcher())))).bind("declWithTagType"),
        this);
    finder.addMatcher(expr(hasType(CreateTagTypeMatcher())).bind("exprWithTagType"), this);
}

void PossibleForwardDeclarationRule::run(const MatchFinder::MatchResult& result)
{
    m_sourceManager = result.SourceManager;

    const DeclRefExpr* declarationReferenceExpression = result.Nodes.getNodeAs<DeclRefExpr>("declRefExpr");
    if (declarationReferenceExpression != nullptr)
        return HandleDeclarationReferenceExpression(declarationReferenceExpression);

    const CXXRecordDecl* recordDeclaration = result.Nodes.getNodeAs<CXXRecordDecl>("recordDecl");
    if (recordDeclaration != nullptr)
        return HandleRecordDeclaration(recordDeclaration);

    const TagDecl* tagDeclaration = result.Nodes.getNodeAs<TagDecl>("tagDecl");
    if (tagDeclaration == nullptr)
        return;

    if (IsInBlacklistedProjectHeader(tagDeclaration))
        return; // already blacklisted, so no point of checking further

    if (! IsInDirectlyIncludedProjectHeader(tagDeclaration))
        return;

    if (! IsForwardDeclarationPossible(tagDeclaration))
    {
        BlacklistIncludedProjectHeader(tagDeclaration);
        return;
    }

    tagDeclaration = tagDeclaration->getCanonicalDecl();

    bool isPointerOrReferenceType = result.Nodes.getNodeAs<QualType>("ptrOrRefType") != nullptr;

    const Decl* declarationWithTagType = result.Nodes.getNodeAs<Decl>("declWithTagType");
    if (declarationWithTagType != nullptr)
        return HandleDeclarationWithTagType(tagDeclaration, declarationWithTagType, isPointerOrReferenceType);

    const Expr* expressionWithTagType = result.Nodes.getNodeAs<Expr>("exprWithTagType");
    if (expressionWithTagType != nullptr)
        return HandleExpressionWithTagType(tagDeclaration, expressionWithTagType);
}

bool PossibleForwardDeclarationRule::IsInBlacklistedProjectHeader(const Decl* declaration)
{
    FileID tagDeclarationFileID = m_sourceManager->getFileID(declaration->getLocation());
    bool result = m_blacklistedProjectHeaders.count(tagDeclarationFileID) > 0;
    return result;
}

void PossibleForwardDeclarationRule::BlacklistIncludedProjectHeader(const Decl* declaration)
{
    FileID tagDeclarationFileID = m_sourceManager->getFileID(declaration->getLocation());
    m_blacklistedProjectHeaders.insert(tagDeclarationFileID);
}
bool PossibleForwardDeclarationRule::IsInDirectlyIncludedProjectHeader(const Decl* declaration)
{
    SourceLocation tagDeclarationLocation = declaration->getLocation();
    if (! m_context.sourceLocationHelper.IsLocationInProjectSourceFile(tagDeclarationLocation, *m_sourceManager))
        return false;

    FileID tagDeclarationFileID = m_sourceManager->getFileID(declaration->getLocation());
    SourceLocation tagDeclarationIncludeLocation = m_sourceManager->getIncludeLoc(tagDeclarationFileID);
    FileID tagDeclarationIncludeFileID = m_sourceManager->getFileID(tagDeclarationIncludeLocation);
    FileID mainFileID = m_context.sourceLocationHelper.GetMainFileID(*m_sourceManager);

    // we want declaration from directly included project header (no indirect dependencies)
    return tagDeclarationFileID != mainFileID && tagDeclarationIncludeFileID == mainFileID;
}

bool PossibleForwardDeclarationRule::IsForwardDeclarationPossible(const TagDecl* tagDeclaration)
{
    // already forward declared
    if (tagDeclaration->getDefinition() == nullptr)
        return false;

    // only enum classes can be forward-declared
    const EnumDecl* enumDeclaration = llvm::dyn_cast_or_null<EnumDecl>(tagDeclaration);
    if (enumDeclaration != nullptr && !enumDeclaration->isScopedUsingClassTag())
        return false;

    // ignore template specializations
    if (llvm::isa<ClassTemplateSpecializationDecl>(tagDeclaration))
        return false;

    // other cases seem ok
    return true;
}

void PossibleForwardDeclarationRule::HandleDeclarationReferenceExpression(const DeclRefExpr* declarationReferenceExpression)
{
    SourceLocation location = declarationReferenceExpression->getLocation();
    if (! m_context.sourceLocationHelper.IsLocationOfInterest(GetName(), location, *m_sourceManager))
        return;

    const Decl* declaration = declarationReferenceExpression->getDecl();
    if (IsInDirectlyIncludedProjectHeader(declaration))
    {
        BlacklistIncludedProjectHeader(declaration);
    }
}

void PossibleForwardDeclarationRule::HandleRecordDeclaration(const CXXRecordDecl* recordDeclaration)
{
    SourceLocation location = recordDeclaration->getLocation();
    if (! m_context.sourceLocationHelper.IsLocationOfInterest(GetName(), location, *m_sourceManager))
        return;

    const CXXRecordDecl* recordDefinition = recordDeclaration->getDefinition();
    if (recordDefinition == nullptr)
        return;

    for (const auto& base : recordDefinition->bases())
    {
        const RecordType* baseRecordType = base.getType()->getAs<RecordType>();
        if (baseRecordType == nullptr)
            continue;

        const RecordDecl* baseDeclaration = baseRecordType->getDecl();
        if (baseDeclaration == nullptr)
            continue;

        const RecordDecl* baseDefinition = baseDeclaration->getDefinition();
        if (baseDefinition == nullptr)
            continue;


        if (IsInDirectlyIncludedProjectHeader(baseDefinition))
        {
            BlacklistIncludedProjectHeader(baseDefinition);
        }
    }
}

void PossibleForwardDeclarationRule::HandleDeclarationWithTagType(const TagDecl* tagDeclaration,
                                                                  const Decl* declarationWithTagType,
                                                                  bool isPointerOrReferenceType)
{
    SourceLocation location = declarationWithTagType->getLocation();
    if (! m_context.sourceLocationHelper.IsLocationOfInterest(GetName(), location, *m_sourceManager))
        return;

    if (isPointerOrReferenceType &&
        (llvm::isa<ParmVarDecl>(declarationWithTagType) ||
         llvm::isa<FieldDecl>(declarationWithTagType) ||
         llvm::isa<FunctionDecl>(declarationWithTagType)))
    {
        if (m_candidateForwardDeclarations.count(tagDeclaration) == 0)
        {
            m_candidateForwardDeclarations.insert(std::make_pair(tagDeclaration, location));
        }
    }
    else
    {
        m_candidateForwardDeclarations.erase(tagDeclaration);
        BlacklistIncludedProjectHeader(tagDeclaration);
    }
}

void PossibleForwardDeclarationRule::HandleExpressionWithTagType(const TagDecl* tagDeclaration,
                                                                 const Expr* expressionWithTagType)
{
    SourceLocation location = expressionWithTagType->getLocStart();
    if (! m_context.sourceLocationHelper.IsLocationOfInterest(GetName(), location, *m_sourceManager))
        return;

    m_candidateForwardDeclarations.erase(tagDeclaration);
    BlacklistIncludedProjectHeader(tagDeclaration);
}

void PossibleForwardDeclarationRule::onEndOfTranslationUnit()
{
    using DeclarationInfoPair = std::pair<const TagDecl*, SourceLocation>;
    std::vector<DeclarationInfoPair> forwardDeclarations;
    for (const DeclarationInfoPair& forwardDeclaration : m_candidateForwardDeclarations)
    {
        if (! IsInBlacklistedProjectHeader(forwardDeclaration.first))
        {
            forwardDeclarations.push_back(forwardDeclaration);
        }
    }

    std::sort(forwardDeclarations.begin(),
              forwardDeclarations.end(),
              [this](const DeclarationInfoPair& left, const DeclarationInfoPair& right)
              {
                  return m_sourceManager->isBeforeInTranslationUnit(left.second, right.second);
              });

    for (const DeclarationInfoPair& forwardDeclaration : forwardDeclarations)
    {
        m_context.outputPrinter->PrintRuleViolation(
            "possible forward declaration",
            Severity::Information,
            boost::str(boost::format("%s '%s' can be forward declared instead of #included")
                % GetTagTypeString(forwardDeclaration.first)
                % forwardDeclaration.first->getQualifiedNameAsString()),
            forwardDeclaration.second,
            *m_sourceManager);
    }

    m_candidateForwardDeclarations.clear();
}
