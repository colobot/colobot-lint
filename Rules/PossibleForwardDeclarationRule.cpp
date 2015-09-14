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
#include <iostream>

using namespace clang;
using namespace clang::ast_matchers;


internal::Matcher<Decl> CreateTagDeclarationMatcher()
{
    return tagDecl(unless(anyOf(isImplicit(), isExpansionInSystemHeader()))).bind("tagDecl");
}

internal::Matcher<QualType> CreateTagTypeMatcher()
{
    return anyOf(qualType(pointerType(pointee(qualType(hasDeclaration(CreateTagDeclarationMatcher())))).bind("pointerType")),
                 qualType(referenceType(pointee(qualType(hasDeclaration(CreateTagDeclarationMatcher())))).bind("referenceType")),
                 qualType(hasDeclaration(CreateTagDeclarationMatcher())));
}

PossibleForwardDeclarationRule::PossibleForwardDeclarationRule(Context& context)
    : ASTCallbackRule(context)
{}

void PossibleForwardDeclarationRule::RegisterASTMatcherCallback(MatchFinder& finder)
{
    finder.addMatcher(valueDecl(hasType(CreateTagTypeMatcher())).bind("declWithTagType"), this);
    finder.addMatcher(expr(hasType(CreateTagTypeMatcher())).bind("exprWithTagType"), this);
}

void PossibleForwardDeclarationRule::run(const MatchFinder::MatchResult& result)
{
    m_sourceManager = result.SourceManager;

    const TagDecl* tagDeclaration = result.Nodes.getNodeAs<TagDecl>("tagDecl");
    if (! IsCandidateForForwardDeclaration(tagDeclaration))
        return;

    tagDeclaration = tagDeclaration->getCanonicalDecl();

    const Type* pointerType = result.Nodes.getNodeAs<Type>("pointerType");
    const Type* referenceType = result.Nodes.getNodeAs<Type>("referenceType");
    bool isPointerOrReferenceType = pointerType != nullptr || referenceType != nullptr;

    const Decl* declarationWithTagType = result.Nodes.getNodeAs<Decl>("declWithTagType");
    if (declarationWithTagType != nullptr)
        return HandleDeclarationWithTagType(tagDeclaration, declarationWithTagType, isPointerOrReferenceType);

    const Expr* expressionWithTagType = result.Nodes.getNodeAs<Expr>("exprWithTagType");
    if (expressionWithTagType != nullptr)
        return HandleExpressionWithTagType(tagDeclaration, expressionWithTagType);
}

bool PossibleForwardDeclarationRule::IsCandidateForForwardDeclaration(const TagDecl* tagDeclaration)
{
    SourceLocation tagDeclarationLocation = tagDeclaration->getLocation();
    if (! m_context.sourceLocationHelper.IsLocationInProjectSourceFile(tagDeclarationLocation, *m_sourceManager))
        return false;

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

    FileID tagDeclarationFileID = m_sourceManager->getFileID(tagDeclaration->getLocation());
    SourceLocation tagDeclarationIncludeLocation = m_sourceManager->getIncludeLoc(tagDeclarationFileID);
    FileID tagDeclarationIncludeFileID = m_sourceManager->getFileID(tagDeclarationIncludeLocation);
    FileID mainFileID = m_context.sourceLocationHelper.GetMainFileID(*m_sourceManager);

    // we want declaration from directly included project header (no indirect dependencies)
    return tagDeclarationFileID != mainFileID && tagDeclarationIncludeFileID == mainFileID;
}

void PossibleForwardDeclarationRule::HandleDeclarationWithTagType(const TagDecl* tagDeclaration,
                                                                  const Decl* declarationWithTagType,
                                                                  bool isPointerOrReferenceType)
{
    SourceLocation location = declarationWithTagType->getLocation();
    if (! m_context.sourceLocationHelper.IsLocationOfInterest(GetName(), location, *m_sourceManager))
        return;

    if (isPointerOrReferenceType &&
        (llvm::isa<ParmVarDecl>(declarationWithTagType) || llvm::isa<FieldDecl>(declarationWithTagType)))
    {
        if (m_candidateForwardDeclarations.count(tagDeclaration) == 0)
        {
            m_candidateForwardDeclarations.insert(std::make_pair(tagDeclaration, location));
        }
    }
    else
    {
        m_candidateForwardDeclarations.erase(tagDeclaration);
    }
}

void PossibleForwardDeclarationRule::HandleExpressionWithTagType(const TagDecl* tagDeclaration,
                                                                 const Expr* expressionWithTagType)
{
    SourceLocation location = expressionWithTagType->getLocStart();
    if (! m_context.sourceLocationHelper.IsLocationOfInterest(GetName(), location, *m_sourceManager))
        return;

    m_candidateForwardDeclarations.erase(tagDeclaration);
}

void PossibleForwardDeclarationRule::onEndOfTranslationUnit()
{
    using DeclarationInfoPair = std::pair<const TagDecl*, SourceLocation>;
    std::vector<DeclarationInfoPair> forwardDeclarations;
    for (const DeclarationInfoPair& forwardDeclaration : m_candidateForwardDeclarations)
    {
        forwardDeclarations.push_back(forwardDeclaration);
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
