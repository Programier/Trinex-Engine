#include <Core/api_object.hpp>
#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <api.hpp>
namespace Engine
{

    REGISTER_CLASS(Engine::ApiObject, Engine::Object);

    // Zero is default or invalid value of ApiObjectNoBase in external API
    constructor_cpp(ApiObjectNoBase)
    {
        _M_ID = 0;
    }


    Identifier ApiObjectNoBase::id() const
    {
        return _M_ID;
    }

    bool ApiObjectNoBase::has_object() const
    {
        return _M_ID != 0;
    }

    bool ApiObjectNoBase::operator==(const ApiObjectNoBase& obj) const
    {
        return _M_ID == obj._M_ID;
    }

    bool ApiObjectNoBase::operator!=(const ApiObjectNoBase& obj) const
    {
        return _M_ID != obj._M_ID;
    }

    bool ApiObjectNoBase::operator<(const ApiObjectNoBase& obj) const
    {
        return _M_ID < obj._M_ID;
    }

    bool ApiObjectNoBase::operator<=(const ApiObjectNoBase& obj) const
    {
        return _M_ID <= obj._M_ID;
    }

    bool ApiObjectNoBase::operator>(const ApiObjectNoBase& obj) const
    {
        return _M_ID > obj._M_ID;
    }

    bool ApiObjectNoBase::operator>=(const ApiObjectNoBase& obj) const
    {
        return _M_ID >= obj._M_ID;
    }

    ApiObjectNoBase::operator Identifier() const
    {
        return _M_ID;
    }

    ApiObjectNoBase& ApiObjectNoBase::destroy()
    {
        if (_M_ID)
        {
            EngineInstance::instance()->api_interface()->destroy_object(_M_ID);
            _M_ID = 0;
        }
        return *this;
    }

    ApiObjectNoBase::~ApiObjectNoBase()
    {
        destroy();
    }
}// namespace Engine
