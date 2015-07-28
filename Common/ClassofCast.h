#pragma once

template<typename DerivedType, typename BaseType> DerivedType* classof_cast(BaseType* ptr)
{
    if (ptr == nullptr)
        return nullptr;

    if (! DerivedType::classof(ptr))
        return nullptr;

    return static_cast<DerivedType*>(ptr);
}
