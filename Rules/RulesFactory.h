#pragma once

#include "../Common/Context.h"
#include "Rule.h"

#include <memory>
#include <vector>

std::vector<std::unique_ptr<Rule>> CreateRules(Context& context);
