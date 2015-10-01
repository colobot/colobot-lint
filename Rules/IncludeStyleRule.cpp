#include "Rules/IncludeStyleRule.h"

#include "Common/Context.h"
#include "Common/OutputPrinter.h"
#include "Common/SourceLocationHelper.h"

#include <clang/Frontend/CompilerInstance.h>
#include <clang/Lex/Preprocessor.h>
#include <clang/Lex/PPCallbacks.h>

#include <set>

#include <boost/format.hpp>

using namespace clang;
using namespace clang::ast_matchers;
using namespace llvm;

class IncludeOrderPPCallbacks : public PPCallbacks
{
public:
    IncludeOrderPPCallbacks(IncludeStyleRule& rule, Context& context, SourceManager& sourceManager)
        : m_rule(rule),
          m_context(context),
          m_sourceManager(sourceManager)
    {}

    void InclusionDirective(SourceLocation hashLoc,
                            const Token& /*includeTok*/,
                            StringRef fileName,
                            bool isAngled,
                            CharSourceRange /*filenameRange*/,
                            const FileEntry* file,
                            StringRef /*searchPath*/,
                            StringRef /*relativePath*/,
                            const Module* /*imported*/) override
    {
        if (file != nullptr &&
            m_context.sourceLocationHelper.IsLocationOfInterest(IncludeStyleRule::GetName(), hashLoc, m_sourceManager))
        {
            std::string fullFileName = m_context.sourceLocationHelper.CleanRawFilename(file->getName());
            m_includeDirectives.emplace_back(hashLoc, fileName.str(), fullFileName, isAngled);
        }
    }

    void EndOfMainFile() override
    {
        m_rule.AtEndOfMainFile(m_includeDirectives, m_sourceManager);
    }

private:
    IncludeStyleRule& m_rule;
    Context& m_context;
    SourceManager& m_sourceManager;
    std::vector<IncludeDirective> m_includeDirectives;
};

/////////////////////////

IncludeStyleRule::IncludeStyleRule(Context& context)
    : Rule(context)
{}

void IncludeStyleRule::RegisterPreProcessorCallbacks(CompilerInstance& compiler)
{
    compiler.getPreprocessor().addPPCallbacks(
        make_unique<IncludeOrderPPCallbacks>(*this, m_context, compiler.getSourceManager()));
}

void IncludeStyleRule::RegisterASTMatcherCallback(MatchFinder& finder)
{
    if (m_context.areWeInFakeHeaderSourceFile)
        finder.addMatcher(recordDecl().bind("recordDecl"), this);
}

void IncludeStyleRule::run(const MatchFinder::MatchResult& result)
{
    const CXXRecordDecl* recordDeclaration = result.Nodes.getNodeAs<CXXRecordDecl>("recordDecl");
    if (recordDeclaration == nullptr)
        return;

    SourceManager& sourceManager = result.Context->getSourceManager();

    SourceLocation location = recordDeclaration->getLocation();
    if (! m_context.sourceLocationHelper.IsLocationOfInterest(GetName(), location, sourceManager))
        return;

    if (recordDeclaration->isImplicit() ||
        !recordDeclaration->isCompleteDefinition() ||
        recordDeclaration->getNumBases() == 0)
    {
        return;
    }

    const CXXBaseSpecifier& mainBase = *recordDeclaration->bases_begin();
    const TypeSourceInfo* typeSourceInfo = mainBase.getTypeSourceInfo();
    if (typeSourceInfo == nullptr)
        return;

    const CXXRecordDecl& baseDecl = *typeSourceInfo->getType()->getAsCXXRecordDecl();
    SourceLocation baseLocation = baseDecl.getLocStart();

    StringRef baseFileName = m_context.sourceLocationHelper.GetCleanFilename(baseLocation, sourceManager);
    if (! IsLocalInclude(baseFileName))
        return;

    m_possibleMainClassBaseIncludes.insert(GetProjectIncludeSubpath(baseFileName));
}

void IncludeStyleRule::AtEndOfMainFile(const std::vector<IncludeDirective>& includeDirectives,
                                       SourceManager& sourceManager)
{
    CheckAngledBrackets(includeDirectives, sourceManager);

    auto currentIt = includeDirectives.begin();
    auto endIt = includeDirectives.end();

    currentIt = CheckFirstInclude(currentIt, endIt, sourceManager);
    currentIt = CheckConfigInclude(currentIt, endIt, sourceManager);
    currentIt = CheckLocalIncludes(currentIt, endIt, sourceManager);
    CheckGlobalIncludes(currentIt, endIt, sourceManager);
}

void IncludeStyleRule::CheckAngledBrackets(const IncludeDirectives& includeDirectives, SourceManager& sourceManager)
{
    for (const auto& include : includeDirectives)
    {
        if (IsLocalInclude(include.fullFileName))
        {
            if (include.isAngled)
            {
                m_context.outputPrinter->PrintRuleViolation(
                    "include style",
                    Severity::Style,
                    boost::str(boost::format("Local include '%s' should be included with quotes, not angled brackets")
                        % include.includeFileName),
                    include.location,
                    sourceManager);
            }
        }
        else
        {
            if (! include.isAngled)
            {
                m_context.outputPrinter->PrintRuleViolation(
                    "include style",
                    Severity::Style,
                    boost::str(boost::format("Global include '%s' should be included with angled brackets, not quotes")
                        % include.includeFileName),
                    include.location,
                    sourceManager);
            }
        }
    }
}

IncludeDirectiveIt IncludeStyleRule::CheckFirstInclude(IncludeDirectiveIt startIt,
                                                       IncludeDirectiveIt endIt,
                                                       SourceManager& sourceManager)
{
    if (startIt == endIt)
        return startIt;

    std::string matchingHeaderInclude = GetMatchingHeaderFileName(sourceManager);

    if (matchingHeaderInclude.empty() &&
        m_possibleMainClassBaseIncludes.count(startIt->includeFileName) == 0)
    {
        return startIt;
    }

    if (!matchingHeaderInclude.empty() &&
        startIt->includeFileName != matchingHeaderInclude)
    {
        m_context.outputPrinter->PrintRuleViolation(
            "include style",
            Severity::Style,
            boost::str(boost::format("Expected first include directive to be"
                " matching header file: '%s', not '%s'")
                % matchingHeaderInclude
                % startIt->includeFileName),
            startIt->location,
            sourceManager);
    }

    ++startIt;
    CheckNewBlock(startIt, endIt, sourceManager);

    return startIt;
}

IncludeDirectiveIt IncludeStyleRule::CheckConfigInclude(IncludeDirectiveIt startIt,
                                                        IncludeDirectiveIt endIt,
                                                        SourceManager& sourceManager)
{
    if (startIt == endIt)
        return startIt;

    auto configIncludeIt = startIt;
    for (; configIncludeIt != endIt; ++configIncludeIt)
    {
        if (IsLocalInclude(configIncludeIt->fullFileName) &&
            StringRef(configIncludeIt->fullFileName).endswith("config.h"))
        {
            break;
        }
    }

    if (configIncludeIt == endIt)
        return startIt;

    if (configIncludeIt != startIt)
    {
        m_context.outputPrinter->PrintRuleViolation(
            "include style",
            Severity::Style,
            boost::str(boost::format("Expected config include directive: '%s', not '%s'")
                % configIncludeIt->includeFileName
                % startIt->includeFileName),
            startIt->location,
            sourceManager);
    }

    ++startIt;
    CheckNewBlock(startIt, endIt, sourceManager);

    return startIt;
}

IncludeDirectiveIt IncludeStyleRule::CheckLocalIncludes(IncludeDirectiveIt startIt,
                                                        IncludeDirectiveIt endIt,
                                                        SourceManager& sourceManager)
{
    if (startIt == endIt)
        return startIt;

    auto endLocalIncludesIt = startIt;
    for (; endLocalIncludesIt != endIt; ++endLocalIncludesIt)
    {
        if (! IsLocalInclude(endLocalIncludesIt->fullFileName))
            break;
    }

    if (startIt == endLocalIncludesIt)
        return startIt;

    for (auto it = startIt; it != endLocalIncludesIt; ++it)
    {
        std::string projectIncludeSubpath = GetProjectIncludeSubpath(it->fullFileName);
        if (it->includeFileName != projectIncludeSubpath)
        {
            m_context.outputPrinter->PrintRuleViolation(
                "include style",
                Severity::Style,
                boost::str(boost::format("Expected local include to be full relative path"
                    " from project local include search path: '%s', not '%s'")
                    % projectIncludeSubpath
                    % it->includeFileName),
                it->location,
                sourceManager);
        }
    }

    CheckIncludeRangeIsSorted(startIt, endLocalIncludesIt, sourceManager);

    CheckNewBlock(endLocalIncludesIt, endIt, sourceManager);

    return endLocalIncludesIt;
}

void IncludeStyleRule::CheckGlobalIncludes(IncludeDirectiveIt startIt,
                                           IncludeDirectiveIt endIt,
                                           SourceManager& sourceManager)
{
    if (startIt == endIt)
        return;

    for (auto it = startIt; it != endIt; ++it)
    {
        if (IsLocalInclude(it->fullFileName))
        {
            m_context.outputPrinter->PrintRuleViolation(
                "include style",
                Severity::Style,
                boost::str(boost::format("Local include '%s' should not be placed after global includes")
                    % it->includeFileName),
                it->location,
                sourceManager);
        }
    }
}

void IncludeStyleRule::CheckNewBlock(IncludeDirectiveIt currentIt,
                                     IncludeDirectiveIt endIt,
                                     SourceManager& sourceManager)
{
    if (currentIt == endIt)
        return;

    auto prevIt = currentIt;
    --prevIt;

    int previousIncludeLineNumber = sourceManager.getPresumedLineNumber(prevIt->location);
    int currentIncludeLineNumber = sourceManager.getPresumedLineNumber(currentIt->location);

    if (currentIncludeLineNumber <= previousIncludeLineNumber + 1)
    {
        m_context.outputPrinter->PrintRuleViolation(
            "include style",
            Severity::Style,
            std::string("Expected empty line between include directives"),
            currentIt->location,
            sourceManager);
    }
}

std::pair<StringRef, StringRef> SplitBySlash(const std::string& path)
{
    auto split = StringRef(path).rsplit('/');
    if (split.second.empty())
        return {StringRef(), split.first};

    return split;
}

void IncludeStyleRule::CheckIncludeRangeIsSorted(IncludeDirectiveIt startIt,
                                                 IncludeDirectiveIt endIt,
                                                 SourceManager& sourceManager)
{
    struct IncludePathComparator
    {
        bool operator()(const std::string& left, const std::string& right) const
        {
            auto leftSplit = SplitBySlash(left);
            auto rightSplit = SplitBySlash(right);

            if (leftSplit.first.empty() && rightSplit.first.empty())
                return leftSplit.second < rightSplit.second;

            if (leftSplit.first.empty() != rightSplit.first.empty())
                return leftSplit.first.empty();

            if (leftSplit.first != rightSplit.first)
                return leftSplit.first < rightSplit.first;

            return leftSplit.second < rightSplit.second;
        }
    };

    std::multiset<std::string, IncludePathComparator> sortedIncludes;
    for (auto it = startIt; it != endIt; ++it)
        sortedIncludes.insert(it->includeFileName);

    auto sortedIt = sortedIncludes.begin();
    auto it = startIt;
    while (it != endIt)
    {
        if (it != startIt)
        {
            auto prevSortedIt = sortedIt;
            --prevSortedIt;
            auto prevSortedItSplit = SplitBySlash(*prevSortedIt);
            auto sortedItSplit = SplitBySlash(*sortedIt);
            if (prevSortedItSplit.first != sortedItSplit.first)
            {
                CheckNewBlock(it, endIt, sourceManager);
            }
        }

        if (it->includeFileName != *sortedIt)
        {
            m_context.outputPrinter->PrintRuleViolation(
                "include style",
                Severity::Style,
                boost::str(boost::format("Broken alphabetical ordering, expected '%s', not '%s'")
                    % *sortedIt
                    % it->includeFileName),
                it->location,
                sourceManager);
            break;
        }

        ++it;
        ++sortedIt;
    }
}

bool IncludeStyleRule::IsLocalInclude(StringRef fileName)
{
    for (const auto& path : m_context.projectLocalIncludePaths)
    {
        if (fileName.startswith(path))
            return true;
    }

    return false;
}

std::string IncludeStyleRule::GetProjectIncludeSubpath(StringRef fileName)
{
    int longestCommonPrefix = 0;
    for (const auto& path : m_context.projectLocalIncludePaths)
    {
        if (fileName.startswith(path))
            longestCommonPrefix = std::max<int>(longestCommonPrefix, path.length());
    }

    if (longestCommonPrefix == 0)
        return "";

    fileName = fileName.drop_front(longestCommonPrefix);
    if (fileName.startswith("/"))
        fileName = fileName.drop_front(1);

    return fileName.str();
}

std::string IncludeStyleRule::GetMatchingHeaderFileName(SourceManager& sourceManager)
{
    if (m_context.areWeInFakeHeaderSourceFile)
        return "";

    StringRef fileName = sourceManager.getFileEntryForID(sourceManager.getMainFileID())->getName();

    if (!fileName.endswith(".cpp"))
        return "";

    std::string matchingHeaderFileName = fileName.drop_back(3).str() + "h";
if (sourceManager.getFileManager().getFile(StringRef(matchingHeaderFileName)) == nullptr)
        return "";

    return GetProjectIncludeSubpath(StringRef(matchingHeaderFileName));
}
