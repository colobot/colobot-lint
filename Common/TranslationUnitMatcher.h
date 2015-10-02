#pragma once

#include <clang/ASTMatchers/ASTMatchers.h>

namespace clang
{
namespace ast_matchers
{
// Named with custom- prefix to avoid clash with Clang 3.7
const internal::VariadicDynCastAllOfMatcher<Decl, TranslationUnitDecl> customTranslationUnitDecl;
} // namespace ast_matchers
} // namespace clang
