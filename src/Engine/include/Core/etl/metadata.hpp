#pragma once
#include <Core/engine_types.hpp>
#include <Core/etl/type_traits.hpp>

namespace Engine
{

    class ENGINE_EXPORT ClassMetaDataHelper
    {
    protected:
        ClassMetaDataHelper(const std::type_index& index, const class Class*);

    public:
        static ENGINE_EXPORT const class Class* find_class(const std::type_index& index);
    };


    template<typename Type>
    class ClassMetaData : public ClassMetaDataHelper
    {
    public:
        static inline const class Class* find_class()
        {
            return ClassMetaDataHelper::find_class(typeid(Type));
        }

        ClassMetaData(const class Class* instance) : ClassMetaDataHelper(std::type_index(typeid(Type)), instance)
        {}
    };
}// namespace Engine
