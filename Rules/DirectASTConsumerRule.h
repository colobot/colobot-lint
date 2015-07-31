#pragma once

#include "Rules/Rule.h"

#include <clang/AST/ASTConsumer.h>

class DirectASTConsumerRule : public Rule,
                              public clang::ASTConsumer
{
public:
    DirectASTConsumerRule(Context& context)
        : Rule(context)
    {}
};
