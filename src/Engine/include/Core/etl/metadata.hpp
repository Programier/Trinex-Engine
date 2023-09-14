#pragma once


namespace Engine
{
    template<typename Type>
    class ClassMetaData
    {
    public:
        static inline const class Class* find_class()
        {
            return Type::static_class_instance();
        }
    };
}// namespace Engine
