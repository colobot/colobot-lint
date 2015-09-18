#pragma once

#include "Rules/Rule.h"

#include <memory>
#include <vector>

std::vector<std::unique_ptr<Rule>> CreateRules(Context& context);
