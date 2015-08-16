#pragma once

#include "Generators/Generator.h"

#include <memory>

std::unique_ptr<Generator> CreateGenerator(Context& context);
