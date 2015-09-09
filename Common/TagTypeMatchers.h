#pragma once

#include <clang/ASTMatchers/ASTMatchers.h>

namespace clang
{
namespace ast_matchers
{
const internal::VariadicDynCastAllOfMatcher<Decl, TagDecl> tagDecl;
AST_TYPE_MATCHER(TagType, tagType);
} // namespace ast_matchers
} // namespace clang
