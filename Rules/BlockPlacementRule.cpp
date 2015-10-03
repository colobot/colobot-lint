#include "Rules/BlockPlacementRule.h"

#include "Common/Context.h"
#include "Common/OutputPrinter.h"
#include "Common/SourceLocationHelper.h"
#include "Common/TranslationUnitMatcher.h"

#include <clang/AST/ASTContext.h>
#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/AST/Decl.h>
#include <clang/AST/Stmt.h>

#include <boost/format.hpp>

using namespace clang;
using namespace clang::ast_matchers;
using namespace llvm;

BlockPlacementRule::BlockPlacementRule(Context& context)
    : Rule(context)
{}

void BlockPlacementRule::RegisterASTMatcherCallback(ast_matchers::MatchFinder& finder)
{
    finder.addMatcher(customTranslationUnitDecl().bind("translationUnit"), this);
}

void BlockPlacementRule::run(const ast_matchers::MatchFinder::MatchResult& result)
{
    const TranslationUnitDecl* translationUnitDeclaration = result.Nodes.getNodeAs<TranslationUnitDecl>("translationUnit");
    if (translationUnitDeclaration == nullptr)
        return;

    m_forbiddenLineNumbers.clear();
    m_reportedLineNumbers.clear();

    m_astContext = result.Context;
    m_mainFileID = m_context.sourceLocationHelper.GetMainFileID(m_astContext->getSourceManager());
    TraverseDecl(const_cast<TranslationUnitDecl*>(translationUnitDeclaration));
}

bool BlockPlacementRule::TraverseDecl(Decl* declaration)
{
    if (declaration != nullptr && !isa<TranslationUnitDecl>(declaration))
    {
        SourceManager& sourceManager = m_astContext->getSourceManager();
        if (sourceManager.getFileID(declaration->getLocation()) != m_mainFileID)
            return true; // there's no point descending into declarations outside main file
    }

    return RecursiveASTVisitor<BlockPlacementRule>::TraverseDecl(declaration);
}

bool BlockPlacementRule::VisitDecl(Decl* declaration)
{
    SourceManager& sourceManager = m_astContext->getSourceManager();

    SourceLocation location = declaration->getLocation();
    if (! m_context.sourceLocationHelper.IsLocationOfInterest(GetName(), location, sourceManager))
        return true; // recurse further

    int declarationStartLineNumber = sourceManager.getPresumedLineNumber(declaration->getLocStart());
    int declarationEndLineNumber = sourceManager.getPresumedLineNumber(declaration->getLocEnd());

    if (m_forbiddenLineNumbers.count(declarationStartLineNumber) > 0 ||
        m_forbiddenLineNumbers.count(declarationEndLineNumber) > 0)
    {
        ReportViolation(declaration->getLocStart(), ViolationType::ClosingBrace /* of previous stmt or decl */);
        return true; // recurse further
    }

    if (! TagDecl::classof(declaration) &&     // class, struct, enum, union, etc.
        ! NamespaceDecl::classof(declaration)) // namespace
    {
        return true; // recurse further
    }

    // one-liners are allowed
    if (declarationStartLineNumber == declarationEndLineNumber)
        return true; // recurse further

    if (! IsDeclarationOpeningBracePlacedCorrectly(declaration->getLocStart(), declaration->getLocEnd()))
        ReportViolation(declaration->getLocStart(), ViolationType::OpeningBrace);

    if (! IsClosingBracePlacedCorrectly(declaration->getLocStart(), declaration->getLocEnd()))
    {
        ReportViolation(declaration->getLocEnd(), ViolationType::ClosingBrace);
        m_reportedLineNumbers.insert(declarationEndLineNumber); // to avoid double errors
    }

    m_forbiddenLineNumbers.insert(declarationEndLineNumber);

    return true; // recurse further
}

bool BlockPlacementRule::VisitStmt(Stmt* statement)
{
    SourceManager& sourceManager = m_astContext->getSourceManager();

    SourceLocation location = statement->getLocStart();
    if (! m_context.sourceLocationHelper.IsLocationOfInterest(GetName(), location, sourceManager))
        return true; // recurse further

    int startLineNumber = sourceManager.getPresumedLineNumber(statement->getLocStart());
    int endLineNumber = sourceManager.getPresumedLineNumber(statement->getLocEnd());

    if (m_forbiddenLineNumbers.count(startLineNumber) > 0 ||
        m_forbiddenLineNumbers.count(endLineNumber) > 0)
    {
        ReportViolation(statement->getLocStart(), ViolationType::ClosingBrace /* of previous stmt or decl */);
        return true; // recurse further
    }

    // one-liners are allowed
    if (startLineNumber == endLineNumber)
        return true; // recurse further

    // compound statement is name for a group of brace-enclosed statement(s)
    //  for example: if (x) { foo(); }
    const CompoundStmt* compountStatment = dyn_cast_or_null<CompoundStmt>(statement);
    if (compountStatment == nullptr)
        return true; // recurse further

    if (! IsStatementOpeningBracePlacedCorrectly(compountStatment->getLBracLoc()))
        ReportViolation(statement->getLocStart(), ViolationType::OpeningBrace);

    if (! IsClosingBracePlacedCorrectly(statement->getLocStart(), statement->getLocEnd()))
    {
        ReportViolation(statement->getLocEnd(), ViolationType::ClosingBrace);
        m_reportedLineNumbers.insert(endLineNumber); // to avoid double errors
    }

    m_forbiddenLineNumbers.insert(endLineNumber);

    return true;
}

bool BlockPlacementRule::IsDeclarationOpeningBracePlacedCorrectly(const SourceLocation& locStart,
                                                                  const SourceLocation& locEnd)
{
    /*
     * In declarations, we scan from beginning of declaration until first opening brace
     *  for example: class Foo ... {
     *               ^             ^
     *             start          end
     */

    int startOffset = m_astContext->getSourceManager().getFileOffset(locStart);
    int endOffset = m_astContext->getSourceManager().getFileOffset(locEnd);
    int scanRange = endOffset - startOffset;

    const char* charData = m_astContext->getSourceManager().getCharacterData(locStart);

    bool haveNewline = false;
    bool haveNonWhitespaceInLine = false;
    for (int i = 0; i <= scanRange; ++i)
    {
        char ch = charData[i];
        if (ch == '{')
        {
            return haveNewline && !haveNonWhitespaceInLine;
        }
        else if (ch == '\n')
        {
            haveNewline = true;
            haveNonWhitespaceInLine = false;
        }
        else if (ch != ' ' && ch != '\t')
        {
            haveNonWhitespaceInLine = true;
        }
    }

    return false;
}

bool BlockPlacementRule::IsStatementOpeningBracePlacedCorrectly(const SourceLocation& openingBraceLocation)
{
    /*
     * In statements, we already know location of opening brace, but we scan backwards to check if there
     * isn't something else before, for example:
     *  while (true)
     *  {
     *  ^       if (...) { ... }
     * start             ^
     * (parent)       opening brace (checked statement)
     */

    int columnNumber = m_astContext->getSourceManager().getPresumedColumnNumber(openingBraceLocation);
    int scanRange = columnNumber;

    const char* charData = m_astContext->getSourceManager().getCharacterData(openingBraceLocation);

    bool haveBrace = false;
    bool haveNonWhitespaceInLine = false;
    for (int i = 0; i <= scanRange; ++i)
    {
        char ch = charData[-i];
        if (ch == '{')
        {
            haveBrace = true;
        }
        else if (ch == '\n')
        {
            return haveBrace && !haveNonWhitespaceInLine;
        }
        else if (ch != ' ' && ch != '\t')
        {
            haveNonWhitespaceInLine = true;
        }
    }

    return false;
}

bool BlockPlacementRule::IsClosingBracePlacedCorrectly(const SourceLocation& locStart,
                                                       const SourceLocation& locEnd)
{
     /*
     * Closing braces are checked like opening braces in statements, by scanning backwards:
     *  class Foo {
     *            ^      int x; };
     *         start             ^
     *                          end
     */

    int startOffset = m_astContext->getSourceManager().getFileOffset(locStart);
    int endOffset = m_astContext->getSourceManager().getFileOffset(locEnd);
    int scanRange = endOffset - startOffset;

    const char* charData = m_astContext->getSourceManager().getCharacterData(locStart);

    bool haveBrace = false;
    bool haveNonWhitespaceInLine = false;
    for (int i = scanRange; i >= 0; --i)
    {
        char ch = charData[i];
        if (ch == '}')
        {
            haveBrace = true;
        }
        else if (ch == '\n' || ch == '{')
        {
            return haveBrace && !haveNonWhitespaceInLine;
        }
        else if (ch != ' ' && ch != '\t')
        {
            haveNonWhitespaceInLine = true;
        }
    }

    return false;
}

void BlockPlacementRule::ReportViolation(const SourceLocation& location, ViolationType type)
{
    int lineNumber = m_astContext->getSourceManager().getPresumedLineNumber(location);
    const char* what = (type == ViolationType::OpeningBrace) ? "begins" : "ends";
    if (m_reportedLineNumbers.count(lineNumber) == 0)
    {
        m_context.outputPrinter->PrintRuleViolation(
            "code block placement",
            Severity::Style,
            boost::str(boost::format("Body of declaration or statement %s in a style that is not allowed")
                % what),
            location,
            m_astContext->getSourceManager());

        m_reportedLineNumbers.insert(lineNumber);
    }
}
