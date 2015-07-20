#include "RulesFactory.h"

#include "NakedDeleteRule.h"
#include "NakedNewRule.h"
#include "TodoRule.h"

#include "BlockPlacementRule.h"

#include "llvm/ADT/STLExtras.h"

using namespace llvm;

std::vector<std::unique_ptr<ASTCallbackRule>> CreateASTRules(Context& context)
{
    std::vector<std::unique_ptr<ASTCallbackRule>> rules;
    rules.push_back(make_unique<NakedDeleteRule>(context));
    rules.push_back(make_unique<NakedNewRule>(context));
    rules.push_back(make_unique<TodoRule>(context));
    return rules;
}

std::vector<std::unique_ptr<DirectASTConsumerRule>> CreateDirectASTConsumerRules(Context& context)
{
    std::vector<std::unique_ptr<DirectASTConsumerRule>> rules;
    rules.push_back(make_unique<BlockPlacementRule>(context));
    return rules;
}
