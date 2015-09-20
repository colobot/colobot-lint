#pragma once

namespace clang
{
class VarDecl;
class ASTContext;
class QualType;
} // namespace clang

bool IsRecordTypeWithoutDataMembers(const clang::QualType& type);

bool IsUninitializedPodVariable(const clang::VarDecl* variableDeclaration, clang::ASTContext* context);
