#include "BlockPlacementRule.h"

#include "../Common/Context.h"
#include "../Common/OutputPrinter.h"
#include "../Common/SourceLocationHelper.h"

#include <clang/AST/ASTContext.h>
#include <clang/AST/Decl.h>
#include <clang/AST/Stmt.h>

using namespace clang;

BlockPlacementRule::BlockPlacementRule(Context& context)
    : DirectASTConsumerRule(context)
    , m_astContext(nullptr)
{}

void BlockPlacementRule::HandleTranslationUnit(ASTContext &context)
{
    m_forbiddenLineNumbers.clear();
    m_reportedLineNumbers.clear();

    TranslationUnitDecl* decl = context.getTranslationUnitDecl();
    m_astContext = &context;
    TraverseDecl(decl);
}

bool BlockPlacementRule::VisitDecl(Decl* declaration)
{
    SourceLocation location = declaration->getLocation();
    if (! m_context.sourceLocationHelper.IsLocationOfInterest(location, m_astContext->getSourceManager()))
        return true; // recurse further

    int declarationStartLineNumber = m_astContext->getSourceManager().getPresumedLineNumber(declaration->getLocStart());
    int declarationEndLineNumber = m_astContext->getSourceManager().getPresumedLineNumber(declaration->getLocEnd());

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
    SourceLocation location = statement->getLocStart();
    if (! m_context.sourceLocationHelper.IsLocationOfInterest(location, m_astContext->getSourceManager()))
        return true; // recurse further

    int startLineNumber = m_astContext->getSourceManager().getPresumedLineNumber(statement->getLocStart());
    int endLineNumber = m_astContext->getSourceManager().getPresumedLineNumber(statement->getLocEnd());

    if (m_forbiddenLineNumbers.count(startLineNumber) > 0 ||
        m_forbiddenLineNumbers.count(endLineNumber) > 0)
    {
        ReportViolation(statement->getLocStart(), ViolationType::ClosingBrace /* of previous stmt or decl */);
        return true; // recurse further
    }

    // compound statement is name for a group of brace-enclosed statement(s)
    //  for example: if (x) { foo(); }
    if (! CompoundStmt::classof(statement))
        return true; // recurse further

    // one-liners are allowed
    if (startLineNumber == endLineNumber)
        return true; // recurse further

    CompoundStmt* compoundStatement = static_cast<CompoundStmt*>(statement);

    SourceLocation openingBraceLocation = compoundStatement->getLBracLoc();
    SourceLocation parentScopeStartLocation;

    auto parents = m_astContext->getParents(*statement);
    if (parents.size() > 0)
    {
        SourceRange range = parents.front().getSourceRange();
        if (range.isValid())
            parentScopeStartLocation = range.getBegin();
        else
            parentScopeStartLocation = statement->getLocStart();
    }
    else
    {
        parentScopeStartLocation = statement->getLocStart();
    }

    if (! IsStatementOpeningBracePlacedCorrectly(parentScopeStartLocation, openingBraceLocation))
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
    int startOffset = m_astContext->getSourceManager().getFileOffset(locStart);
    int endOffset = m_astContext->getSourceManager().getFileOffset(locEnd);
    const char* charData = m_astContext->getSourceManager().getCharacterData(locStart);

    /*
     * In declarations, we scan from beginning of declaration until first opening brace
     *  for example: class Foo ... {
     *               ^             ^
     *             start          end
     */

    bool haveNewline = false;
    bool haveNonWhitespaceInLine = false;
    int scanRange = endOffset - startOffset;
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

bool BlockPlacementRule::IsStatementOpeningBracePlacedCorrectly(const SourceLocation& parentStartLocation,
                                                                const SourceLocation& openingBraceLocation)
{
    int startOffset = m_astContext->getSourceManager().getFileOffset(parentStartLocation);
    int endOffset = m_astContext->getSourceManager().getFileOffset(openingBraceLocation);
    const char* charData = m_astContext->getSourceManager().getCharacterData(parentStartLocation);

    /*
     * In statements, we already know location of opening brace, but we scan backwards to check if there
     *  isn't something else before, for example:
     *  while (true)
     *  {
     *  ^       if (...) { ... }
     * start             ^
     * (parent)       opening brace (checked statement)
     */

    bool haveBrace = false;
    bool haveNonWhitespaceInLine = false;
    int scanRange = endOffset - startOffset;
    for (int i = scanRange; i >= 0; --i)
    {
        char ch = charData[i];
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
    int startOffset = m_astContext->getSourceManager().getFileOffset(locStart);
    int endOffset = m_astContext->getSourceManager().getFileOffset(locEnd);
    const char* charData = m_astContext->getSourceManager().getCharacterData(locStart);

     /*
     * Closing braces are checked like opening braces in statements, by scanning backwards:
     *  class Foo {
     *            ^      int x; };
     *         start             ^
     *                          end
     */

    bool haveBrace = false;
    bool haveNonWhitespaceInLine = false;
    int scanRange = endOffset - startOffset;
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
        m_context.printer.PrintRuleViolation(
            "code block placement",
            Severity::Style,
            std::string("Body of declaration or statement ") + what + " in a style that is not allowed",
            location,
            m_astContext->getSourceManager());

        m_reportedLineNumbers.insert(lineNumber);
    }
}
