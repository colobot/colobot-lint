#include "Common/UninitializedPodVariableHelper.h"


#include <clang/AST/Decl.h>
#include <clang/AST/DeclCXX.h>
#include <clang/AST/Expr.h>
#include <clang/AST/ExprCXX.h>

using namespace clang;

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

} // anonymous namespace

bool IsUninitializedPodVariable(const VarDecl* variableDeclaration, ASTContext* context)
{
    QualType type = variableDeclaration->getType();
    if (! type.isPODType(*context))
        return false;

    return (!variableDeclaration->hasInit()) ||
           (type->isRecordType() && HasImplicitInitialization(variableDeclaration));
}
