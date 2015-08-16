#pragma once

#include "Common/Severity.h"

#include <tinyxml.h>

#include <iosfwd>
#include <memory>
#include <string>
#include <unordered_set>

namespace clang
{
class SourceLocation;
class SourceManager;
} // namespace clang


enum class OutputType
{
    CppcheckReport,
    DotGraph
};

struct DotGraphEdge
{
    std::string source;
    std::string target;
    std::string options;

    bool operator==(const DotGraphEdge& other) const
    {
        return source == other.source &&
               target == other.target &&
               options == other.options;
    }
};

namespace std
{
template<>
struct hash<DotGraphEdge>
{
    std::size_t operator()(const DotGraphEdge& edge) const
    {
        auto sourceHash = std::hash<std::string>()(edge.source);
        auto targetHash = std::hash<std::string>()(edge.target);
        auto optionsHash = std::hash<std::string>()(edge.options);
        return (sourceHash << 12) | (targetHash << 6) | optionsHash;
    }
};
}

class OutputPrinter
{
public:
    OutputPrinter(const std::string& outputFileName, OutputType type);

    void PrintRuleViolation(const std::string& ruleName,
                            Severity severity,
                            const std::string& description,
                            clang::SourceLocation location,
                            clang::SourceManager& sourceManager,
                            int lineOffset = 0);

    void PrintRuleViolation(const std::string& ruleName,
                            Severity severity,
                            const std::string& description,
                            const std::string& fileName,
                            int lineNumber);

    void PrintGraphEdge(const std::string& source,
                        const std::string& destination,
                        const std::string& options = "");

    void Save();

private:
    void InitCppcheckReport();
    void SaveCppcheckReport();
    void SaveDotGraph();
    void SaveDotGraph(std::ostream& str);
    std::string GetSeverityString(Severity severity);

private:
    const OutputType m_type;
    const std::string m_outputFileName;
    TiXmlDocument m_document;
    std::unique_ptr<TiXmlElement> m_resultsElement;
    std::unique_ptr<TiXmlElement> m_errorsElement;
    std::unordered_set<DotGraphEdge> m_graphEdges;
};
