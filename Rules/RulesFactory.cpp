#include "RulesFactory.h"

#include "NakedDeleteRule.h"
#include "NakedNewRule.h"
#include "TodoRule.h"

#include "llvm/ADT/STLExtras.h"

using namespace llvm;

std::vector<std::unique_ptr<Rule>> CreateRules(Context& context)
{
    std::vector<std::unique_ptr<Rule>> rules;
    rules.push_back(make_unique<NakedDeleteRule>(context));
    rules.push_back(make_unique<NakedNewRule>(context));
    rules.push_back(make_unique<TodoRule>(context));
    return rules;
}
