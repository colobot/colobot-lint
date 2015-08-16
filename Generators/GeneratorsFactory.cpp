#include "Generators/GeneratorsFactory.h"

#include "Common/Context.h"

#include "Generators/DependencyGraphGenerator.h"
#include "Generators/DeploymentGraphGenerator.h"

using namespace llvm;

std::unique_ptr<Generator> CreateGenerator(Context& context)
{
    if (context.generatorSelection == DependencyGraphGenerator::GetName())
        return make_unique<DependencyGraphGenerator>(context);
    else if (context.generatorSelection == DeploymentGraphGenerator::GetName())
        return make_unique<DeploymentGraphGenerator>(context);
    return nullptr;
}
