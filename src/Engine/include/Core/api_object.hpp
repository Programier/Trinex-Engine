#pragma once
#include <Core/engine_types.hpp>
#include <Core/object.hpp>

namespace Engine
{
    class ENGINE_EXPORT ApiObjectNoBase
    {
    protected:
        Identifier _M_ID = 0;


    public:
        ApiObjectNoBase();
        delete_copy_constructors(ApiObjectNoBase);

        Identifier id() const;
        bool has_object() const;
        bool operator==(const ApiObjectNoBase& obj) const;
        bool operator!=(const ApiObjectNoBase& obj) const;
        bool operator<(const ApiObjectNoBase& obj) const;
        bool operator<=(const ApiObjectNoBase& obj) const;
        bool operator>(const ApiObjectNoBase& obj) const;
        bool operator>=(const ApiObjectNoBase& obj) const;

        operator Identifier() const;
        ApiObjectNoBase& destroy();

    protected:
        virtual ~ApiObjectNoBase();
    };

    class ENGINE_EXPORT ApiObject : public Object, public ApiObjectNoBase
    {
    public:
        declare_class(ApiObject, Object);
    };


}// namespace Engine
