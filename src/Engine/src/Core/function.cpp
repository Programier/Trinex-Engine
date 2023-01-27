#include <Core/function.hpp>

namespace Engine
{
    IFunction& IFunction::base_name(const String& _base_name)
    {
        _M_base_name = _base_name;
        auto pos = _M_prototype_name.find_first_of('*');
        if (pos != _M_prototype_name.length())
            _M_name = _M_prototype_name.replace(pos, 1, _base_name);
        return *this;
    }

    const String& IFunction::base_name() const
    {
        return _M_base_name;
    }
    const String& IFunction::prototype_name() const
    {
        return _M_prototype_name;
    }
    const String& IFunction::name() const
    {
        return _M_name;
    }
}// namespace Engine
