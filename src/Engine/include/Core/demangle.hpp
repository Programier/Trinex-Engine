#pragma once
#include <Core/engine_types.hpp>
#include <Core/export.hpp>
#include <typeinfo>
#include <Core/predef.hpp>
#include <Core/class.hpp>
#include <Core/etl/metadata.hpp>
#include <Core/etl/type_traits.hpp>

namespace Engine::Demangle
{
    ENGINE_EXPORT String decode_name(const std::type_info& info);
    ENGINE_EXPORT String decode_name(const String& name);

    template<typename Type>
    FORCE_INLINE typename std::enable_if<is_object_based<Type>::value, String>::type name_of_type()
    {
        return ClassMetaData<Type>::find_class()->name();
    }

    template<typename Type>
    FORCE_INLINE typename std::enable_if<!is_object_based<Type>::value, String>::type name_of_type()
    {
        return decode_name(typeid(Type));
    }
}// namespace Engine
