#pragma once
#include <Core/etl/type_traits.hpp>

namespace Engine
{
    template<typename Type>
    class ClassMetaDataBase {
        static const class Class* const class_instance;
    public:
        static inline const class Class* find_class()
        {
            return class_instance;
        }
    };

    template<typename Type>
    using ClassMetaData = ClassMetaDataBase<typename add_pointer_unique<Type>::type>;

}// namespace Engine
