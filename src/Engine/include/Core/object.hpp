#pragma once
#include <Core/engine_types.hpp>
#include <Core/export.hpp>

namespace Engine
{
    CLASS Object
    {
    protected:
        ObjID _M_ID = 0;

    public:
        Object();
        Object(const Object&);
        Object(Object&& obj);
        Object& operator=(Object&& obj);
        Object& operator=(const Object&);
        ObjID id() const;
        bool has_object() const;
        bool operator==(const Object& obj);
        bool operator!=(const Object& obj);
        Object& destroy();

    protected:
        virtual ~Object();
    };


}// namespace Engine
