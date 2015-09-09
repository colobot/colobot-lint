#pragma once

#include <clang/AST/Decl.h>

inline const char* GetTagTypeString(const clang::TagDecl* tagDeclaration)
{
    if (tagDeclaration->isEnum())
    {
        const clang::EnumDecl* enumDecl = static_cast<const clang::EnumDecl*>(tagDeclaration);
        return enumDecl->isScopedUsingClassTag() ? "Enum class" : "Enum";
    }
    else if (tagDeclaration->isClass())
        return "Class";
    else if (tagDeclaration->isUnion())
        return "Union";
    else if (tagDeclaration->isStruct())
        return "Struct";
    return "";
}

inline const char* GetLowercaseTagTypeString(const clang::TagDecl* tagDeclaration)
{
    if (tagDeclaration->isEnum())
    {
        const clang::EnumDecl* enumDecl = static_cast<const clang::EnumDecl*>(tagDeclaration);
        return enumDecl->isScopedUsingClassTag() ? "enum class" : "enum";
    }
    if (tagDeclaration->isClass())
        return "class";
    else if (tagDeclaration->isUnion())
        return "union";
    else if (tagDeclaration->isStruct())
        return "struct";
    return "";
}
