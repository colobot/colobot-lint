#include "BlockPlacementRule.h"

#include "../Common/SourceLocationHelper.h"

#include "clang/AST/Decl.h"
#include "clang/AST/Stmt.h"

using namespace clang;

BlockPlacementRule::BlockPlacementRule(Context& context)
    : DirectASTConsumerRule(context)
    , m_astContext(nullptr)
{}

void BlockPlacementRule::HandleTranslationUnit(clang::ASTContext &context)
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

    int declarationStartLineNumber = m_astContext->getSourceManager().getSpellingLineNumber(declaration->getLocStart());
    int declarationEndLineNumber = m_astContext->getSourceManager().getSpellingLineNumber(declaration->getLocEnd());

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

    if (! IsOpeningBracePlacedCorrectly(declaration->getLocStart(), declaration->getLocEnd()))
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

    int startLineNumber = m_astContext->getSourceManager().getSpellingLineNumber(statement->getLocStart());
    int endLineNumber = m_astContext->getSourceManager().getSpellingLineNumber(statement->getLocEnd());

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

    SourceLocation scanStartLocation;

    auto parents = m_astContext->getParents(*statement);
    if (parents.size() > 0)
    {
        SourceRange range = parents.front().getSourceRange();
        if (range.isValid())
            scanStartLocation = range.getBegin();
        else
            scanStartLocation = statement->getLocStart();
    }
    else
    {
        scanStartLocation = statement->getLocStart();
    }

    if (! IsOpeningBracePlacedCorrectly(scanStartLocation, statement->getLocEnd()))
        ReportViolation(statement->getLocStart(), ViolationType::OpeningBrace);

    if (! IsClosingBracePlacedCorrectly(scanStartLocation, statement->getLocEnd()))
    {
        ReportViolation(statement->getLocEnd(), ViolationType::ClosingBrace);
        m_reportedLineNumbers.insert(endLineNumber); // to avoid double errors
    }

    m_forbiddenLineNumbers.insert(endLineNumber);

    return true;
}

bool BlockPlacementRule::IsOpeningBracePlacedCorrectly(const SourceLocation& locStart,
                                                       const SourceLocation& locEnd)
{
    int startOffset = m_astContext->getSourceManager().getFileOffset(locStart);
    int endOffset = m_astContext->getSourceManager().getFileOffset(locEnd);

    const char* charData = m_astContext->getSourceManager().getCharacterData(locStart);

    bool haveNewline = false;
    bool haveNonWhitespaceInLine = false;
    int scanRange = endOffset - startOffset;
    for (int i = 0; i <= scanRange; ++i)
    {
        char ch = charData[i];
        if (ch == '{')
        {
            return (haveNewline) && !haveNonWhitespaceInLine;
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

bool BlockPlacementRule::IsClosingBracePlacedCorrectly(const SourceLocation& locStart,
                                                       const SourceLocation& locEnd)
{
    int startOffset = m_astContext->getSourceManager().getFileOffset(locStart);
    int endOffset = m_astContext->getSourceManager().getFileOffset(locEnd);

    const char* charData = m_astContext->getSourceManager().getCharacterData(locStart);

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

void BlockPlacementRule::ReportViolation(const clang::SourceLocation location, ViolationType type)
{
    int lineNumber = m_astContext->getSourceManager().getSpellingLineNumber(location);
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
