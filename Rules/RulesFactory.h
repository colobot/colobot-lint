#pragma once

#include "ASTRule.h"
#include "TokenRule.h"

#include <memory>
#include <vector>

std::vector<std::unique_ptr<ASTRule>> CreateASTRules(Context& context);

std::vector<std::unique_ptr<TokenRule>> CreateTokenRules(Context& context);
