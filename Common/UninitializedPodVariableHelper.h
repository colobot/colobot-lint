#pragma once

namespace clang
{
class VarDecl;
class ASTContext;
} // namespace clang

bool IsUninitializedPodVariable(const clang::VarDecl* variableDeclaration, clang::ASTContext* context);
