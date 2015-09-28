#pragma once

#include <set>
#include <string>

struct FunctionDefinitionContext
{
    std::set<std::string> definedFunctions;
    std::set<std::string> undefinedFunctions;
};
