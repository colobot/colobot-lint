#pragma once

#include "../Common/Context.h"

class Rule
{
public:
    Rule(Context& context)
        : m_context(context)
    {}

    virtual ~Rule()
    {}

protected:
    Context& m_context;
};
