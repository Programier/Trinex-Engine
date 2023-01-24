#pragma once
#include <Core/engine_types.hpp>
#include <Core/export.hpp>
#include <Core/object.hpp>

namespace Engine
{
    CLASS ApiObject : public virtual Object
    {
    protected:
        ObjID _M_ID = 0;

        declare_instance_info_hpp(ApiObject);

    public:
        constructor_hpp(ApiObject);
        delete_copy_constructors(ApiObject);

        ObjID id() const;
        bool has_object() const;
        bool operator==(const ApiObject& obj) const;
        bool operator!=(const ApiObject& obj) const;
        bool operator<(const ApiObject& obj) const;
        bool operator<=(const ApiObject& obj) const;
        bool operator>(const ApiObject& obj) const;
        bool operator>=(const ApiObject& obj) const;

        operator ObjID() const;
        ApiObject& destroy();

    protected:
        virtual ~ApiObject();
    };


}// namespace Engine
