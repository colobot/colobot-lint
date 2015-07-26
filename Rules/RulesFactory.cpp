#include "RulesFactory.h"

#include "../Common/Context.h"

#include "ClassNamingRule.h"
#include "EnumNamingRule.h"
#include "FunctionNamingRule.h"
#include "InconsistentDeclarationParameterNameRule.h"
#include "NakedDeleteRule.h"
#include "NakedNewRule.h"
#include "OldStyleFunctionRule.h"
#include "TodoRule.h"
#include "UninitializedFieldRule.h"
#include "UninitializedLocalVariableRule.h"
#include "VariableNamingRule.h"

#include "BlockPlacementRule.h"
#include "WhitespaceRule.h"

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
    AddRule<ClassNamingRule>(rules, context);
    AddRule<EnumNamingRule>(rules, context);
    AddRule<UninitializedFieldRule>(rules, context);
    AddRule<OldStyleFunctionRule>(rules, context); // must be first
    AddRule<UninitializedLocalVariableRule>(rules, context); // must be second
    AddRule<InconsistentDeclarationParameterNameRule>(rules, context);
    return rules;
}

std::vector<std::unique_ptr<DirectASTConsumerRule>> CreateDirectASTConsumerRules(Context& context)
{
    std::vector<std::unique_ptr<DirectASTConsumerRule>> rules;
    AddRule<BlockPlacementRule>(rules, context);
    AddRule<WhitespaceRule>(rules, context);
    return rules;
}
