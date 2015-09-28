#include "Rules/RulesFactory.h"

#include "Common/Context.h"

#include "Rules/BlockPlacementRule.h"
#include "Rules/ClassNamingRule.h"
#include "Rules/EnumNamingRule.h"
#include "Rules/FunctionNamingRule.h"
#include "Rules/ImplicitBoolCastRule.h"
#include "Rules/IncludeStyleRule.h"
#include "Rules/InconsistentDeclarationParameterNameRule.h"
#include "Rules/LicenseInHeaderRule.h"
#include "Rules/NakedDeleteRule.h"
#include "Rules/NakedNewRule.h"
#include "Rules/OldStyleFunctionRule.h"
#include "Rules/OldStyleNullPointerRule.h"
#include "Rules/PossibleForwardDeclarationRule.h"
#include "Rules/TodoRule.h"
#include "Rules/UndefinedFunctionRule.h"
#include "Rules/UninitializedFieldRule.h"
#include "Rules/UninitializedLocalVariableRule.h"
#include "Rules/UnusedForwardDeclarationRule.h"
#include "Rules/VariableNamingRule.h"
#include "Rules/WhitespaceRule.h"

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

std::vector<std::unique_ptr<Rule>> CreateRules(Context& context)
{
    std::vector<std::unique_ptr<Rule>> rules;
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
    AddRule<IncludeStyleRule>(rules, context);
    AddRule<ImplicitBoolCastRule>(rules, context);
    AddRule<OldStyleNullPointerRule>(rules, context);
    AddRule<UnusedForwardDeclarationRule>(rules, context);
    AddRule<PossibleForwardDeclarationRule>(rules, context);
    AddRule<BlockPlacementRule>(rules, context);
    AddRule<WhitespaceRule>(rules, context);
    AddRule<LicenseInHeaderRule>(rules, context);
    AddRule<UndefinedFunctionRule>(rules, context);
    return rules;
}