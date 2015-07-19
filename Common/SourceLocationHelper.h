#pragma once

struct Context;

namespace clang
{
class SourceLocation;
class SourceManager;
}

class SourceLocationHelper
{
public:
    void SetContext(Context* context);

    bool IsLocationOfInterest(const clang::SourceLocation& location,
                              clang::SourceManager& sourceManager);

private:
    Context* m_context;
};
