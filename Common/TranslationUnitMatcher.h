#pragma once

#include <clang/ASTMatchers/ASTMatchers.h>

namespace clang
{
namespace ast_matchers
{
const internal::VariadicDynCastAllOfMatcher<Decl, TranslationUnitDecl> translationUnitDecl;
} // namespace ast_matchers
} // namespace clang
