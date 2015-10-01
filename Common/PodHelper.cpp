#include "Common/PodHelper.h"

#include <clang/AST/Decl.h>
#include <clang/AST/DeclCXX.h>
#include <clang/AST/Expr.h>
#include <clang/AST/ExprCXX.h>

using namespace clang;
using namespace llvm;

namespace
{

bool HasImplicitInitialization(const VarDecl* variableDeclaration)
{
    const CXXConstructExpr* constructExpr = dyn_cast_or_null<const CXXConstructExpr>(variableDeclaration->getInit());
    if (constructExpr == nullptr ||
        constructExpr->getConstructor() == nullptr)
    {
        return false;
    }

    return constructExpr->getConstructor()->isImplicit();
}

bool IsRecordWithoutDataMembers(const CXXRecordDecl* recordDeclaration)
{
    if (recordDeclaration == nullptr)
        return false;

    recordDeclaration = recordDeclaration->getDefinition();
    if (recordDeclaration == nullptr)
        return false;

    if (!recordDeclaration->field_empty())
        return false;

    for (const auto& base : recordDeclaration->bases())
    {
        if (!IsRecordTypeWithoutDataMembers(base.getType()))
        return false;
    }

    return true;
}

} // anonymous namespace

bool IsRecordTypeWithoutDataMembers(const QualType& type)
{
    const auto* checkRecordType = type->getAs<RecordType>();
    return checkRecordType != nullptr &&
            IsRecordWithoutDataMembers(
                dyn_cast_or_null<CXXRecordDecl>(checkRecordType->getDecl()));
}

bool IsUninitializedPodVariable(const VarDecl* variableDeclaration, ASTContext* context)
{
    QualType type = variableDeclaration->getType();
    if (! type.isPODType(*context))
        return false;

    if (IsRecordTypeWithoutDataMembers(type))
        return false;

    return (!variableDeclaration->hasInit()) ||
           (type->isRecordType() && HasImplicitInitialization(variableDeclaration));
}
