#pragma once
#include <Core/engine_types.hpp>
#include <Core/etl/type_traits.hpp>
#include <Core/export.hpp>

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
    class ClassMetaDataBase : public ClassMetaDataHelper
    {
    public:
        static inline const class Class* find_class()
        {
            return ClassMetaDataHelper::find_class(typeid(Type));
        }

        ClassMetaDataBase(const class Class* instance) : ClassMetaDataHelper(std::type_index(typeid(Type)), instance)
        {}
    };


    template<typename Type>
    using ClassMetaData = ClassMetaDataBase<Type>;

    template<typename Type>
    ClassMetaData<Type> trinex_metaclass_database;

}// namespace Engine
