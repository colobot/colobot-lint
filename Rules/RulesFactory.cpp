#include "RulesFactory.h"

#include "NakedDeleteRule.h"
#include "NakedNewRule.h"
#include "TodoRule.h"

#include "BraceInNewLineRule.h"

#include "llvm/ADT/STLExtras.h"

using namespace llvm;

std::vector<std::unique_ptr<ASTRule>> CreateASTRules(Context& context)
{
    std::vector<std::unique_ptr<ASTRule>> rules;
    rules.push_back(make_unique<NakedDeleteRule>(context));
    rules.push_back(make_unique<NakedNewRule>(context));
    rules.push_back(make_unique<TodoRule>(context));
    return rules;
}

std::vector<std::unique_ptr<TokenRule>> CreateTokenRules(Context& context)
{
    std::vector<std::unique_ptr<TokenRule>> rules;
    rules.push_back(make_unique<BraceInNewLineRule>(context));
    return rules;
}
