#include "Rules/UnusedForwardDeclarationRule.h"

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

UnusedForwardDeclarationRule::UnusedForwardDeclarationRule(Context& context)
    : Rule(context)
{}

void UnusedForwardDeclarationRule::RegisterASTMatcherCallback(MatchFinder& finder)
{
    finder.addMatcher(tagDecl(unless(isImplicit())).bind("tagDecl"), this);
    finder.addMatcher(tagType(hasDeclaration(tagDecl().bind("tagDecl"))).bind("tagType"), this);
}

void UnusedForwardDeclarationRule::run(const MatchFinder::MatchResult& result)
{
    const TagDecl* tagDeclaration = result.Nodes.getNodeAs<TagDecl>("tagDecl");
    const TagType* tagTypeReference = result.Nodes.getNodeAs<TagType>("tagType");
    if (tagDeclaration == nullptr)
        return;

    m_sourceManager = result.SourceManager;

    SourceLocation location = tagDeclaration->getLocation();
    if (! m_context.sourceLocationHelper.IsLocationOfInterest(GetName(), location, *m_sourceManager))
        return;

    if (tagTypeReference != nullptr)
        return HandleTypeReference(tagDeclaration);

    else if (tagDeclaration->isThisDeclarationADefinition() ||
             llvm::isa<ClassTemplateSpecializationDecl>(tagDeclaration))
        return HandleDefinition(tagDeclaration);
    else
        return HandleForwardDeclaration(tagDeclaration);
}

void UnusedForwardDeclarationRule::HandleDefinition(const TagDecl* tagDeclaration)
{
    const TagDecl* canonicalDeclaration = tagDeclaration->getCanonicalDecl();
    m_definedDeclarations.insert(canonicalDeclaration);
    m_usesOfForwardDeclarations.erase(canonicalDeclaration);
}

void UnusedForwardDeclarationRule::HandleForwardDeclaration(const TagDecl* tagDeclaration)
{
    const TagDecl* canonicalDeclaration = tagDeclaration->getCanonicalDecl();

    if (m_definedDeclarations.count(canonicalDeclaration) > 0)
    {
        m_context.outputPrinter->PrintRuleViolation(
            "unused forward declaration",
            Severity::Information,
            boost::str(boost::format("Redundant forward declaration after definition of %s '%s'")
                % GetLowercaseTagTypeString(tagDeclaration)
                % tagDeclaration->getQualifiedNameAsString()),
            tagDeclaration->getLocation(),
            *m_sourceManager);
        return;
    }

    if (m_usesOfForwardDeclarations.count(canonicalDeclaration) > 0)
    {
        m_context.outputPrinter->PrintRuleViolation(
            "unused forward declaration",
            Severity::Information,
            boost::str(boost::format("Repeated forward declaration of %s '%s'")
                % GetLowercaseTagTypeString(tagDeclaration)
                % tagDeclaration->getQualifiedNameAsString()),
            tagDeclaration->getLocation(),
            *m_sourceManager);
        return;
    }

    m_usesOfForwardDeclarations.insert(std::make_pair(canonicalDeclaration,
                                                      ForwardDeclarationUseInfo(tagDeclaration)));
}

void UnusedForwardDeclarationRule::HandleTypeReference(const TagDecl* tagDeclaration)
{
    const TagDecl* canonicalDeclaration = tagDeclaration->getCanonicalDecl();

    if (m_definedDeclarations.count(canonicalDeclaration) > 0)
    {
        m_usesOfForwardDeclarations.erase(canonicalDeclaration);
    }
    else
    {
        m_usesOfForwardDeclarations[canonicalDeclaration].useCount++;
    }
}

void UnusedForwardDeclarationRule::onEndOfTranslationUnit()
{
    std::vector<const TagDecl*> unusedDeclarations;
    unusedDeclarations.reserve(m_usesOfForwardDeclarations.size());

    for (const auto& forwardDeclarationUse : m_usesOfForwardDeclarations)
    {
        int minimumUseCount = llvm::isa<EnumDecl>(forwardDeclarationUse.first) ? 1 : 0;

        if (forwardDeclarationUse.second.useCount == minimumUseCount)
        {
            unusedDeclarations.push_back(forwardDeclarationUse.second.forwardDeclaration);
        }
    }
    std::sort(unusedDeclarations.begin(),
              unusedDeclarations.end(),
              [this](const TagDecl* left, const TagDecl* right)
              {
                  return m_sourceManager->isBeforeInTranslationUnit(left->getLocation(),
                                                                    right->getLocation());
              });

    for (const TagDecl* unusedDeclaration : unusedDeclarations)
    {
        m_context.outputPrinter->PrintRuleViolation(
            "unused forward declaration",
            Severity::Information,
            boost::str(boost::format("Unused forward declaration of %s '%s'")
                % GetLowercaseTagTypeString(unusedDeclaration)
                % unusedDeclaration->getQualifiedNameAsString()),
            unusedDeclaration->getLocation(),
            *m_sourceManager);
    }

    m_definedDeclarations.clear();
    m_usesOfForwardDeclarations.clear();
}
