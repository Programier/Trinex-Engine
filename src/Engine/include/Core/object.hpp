#pragma once
#include <Core/engine_types.hpp>
#include <Core/export.hpp>
#include <Core/instance.hpp>

namespace Engine
{
    CLASS Object : public virtual ObjectInstance
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
        bool operator==(const Object& obj) const;
        bool operator!=(const Object& obj) const;
        bool operator < (const Object& obj) const;
        bool operator <= (const Object& obj) const;
        bool operator > (const Object& obj) const;
        bool operator >= (const Object& obj) const;

        operator ObjID() const;
        Object& destroy();

    protected:
        virtual ~Object();
    };


}// namespace Engine
