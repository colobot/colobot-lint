#include "RulesFactory.h"

#include "../Common/Context.h"

#include "NakedDeleteRule.h"
#include "NakedNewRule.h"
#include "TodoRule.h"
#include "VariableNamingRule.h"
#include "FunctionNamingRule.h"

#include "BlockPlacementRule.h"

#include <llvm/ADT/STLExtras.h>

#include <iostream>

using namespace llvm;

template<typename RuleType, typename Container>
void AddRule(Container& rules, Context& context)
{
    std::string ruleName = RuleType::GetName();

    if (context.rulesSelection.empty() ||
        context.rulesSelection.count(ruleName) > 0)
    {
        if (context.debug)
        {
            std::cerr << "Using rule " << ruleName << std::endl;
        }
        rules.push_back(make_unique<RuleType>(context));
    }
    else
    {
        if (context.debug)
        {
            std::cerr << "Skipping rule " << ruleName << std::endl;
        }
    }
}

std::vector<std::unique_ptr<ASTCallbackRule>> CreateASTRules(Context& context)
{
    std::vector<std::unique_ptr<ASTCallbackRule>> rules;
    AddRule<NakedDeleteRule>(rules, context);
    AddRule<NakedNewRule>(rules, context);
    AddRule<TodoRule>(rules, context);
    AddRule<VariableNamingRule>(rules, context);
    AddRule<FunctionNamingRule>(rules, context);
    return rules;
}

std::vector<std::unique_ptr<DirectASTConsumerRule>> CreateDirectASTConsumerRules(Context& context)
{
    std::vector<std::unique_ptr<DirectASTConsumerRule>> rules;
    AddRule<BlockPlacementRule>(rules, context);
    return rules;
}
