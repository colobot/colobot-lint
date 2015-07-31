#pragma once

#include "Rules/ASTCallbackRule.h"
#include "Rules/DirectASTConsumerRule.h"

#include <memory>
#include <vector>

std::vector<std::unique_ptr<ASTCallbackRule>> CreateASTRules(Context& context);

std::vector<std::unique_ptr<DirectASTConsumerRule>> CreateDirectASTConsumerRules(Context& context);
