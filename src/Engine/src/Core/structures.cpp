#include <Core/structures.hpp>

namespace Engine
{
    const BindLocation BindLocation::undefined = BindLocation(255, 255);

    BindLocation::BindLocation() : BindLocation(255, 255)
    {}

    BindLocation::BindLocation(BindingIndex in_binding, BindingIndex in_set) : binding(in_binding), set(in_set)
    {}

    bool BindLocation::operator==(const BindLocation& location) const
    {
        return location.id == id;
    }

    bool BindLocation::operator!=(const BindLocation& location) const
    {
        return location.id != id;
    }

    bool BindLocation::operator<(const BindLocation& location) const
    {
        return id < location.id;
    }

    bool BindLocation::operator<=(const BindLocation& location) const
    {
        return id <= location.id;
    }

    bool BindLocation::operator>(const BindLocation& location) const
    {
        return id > location.id;
    }

    bool BindLocation::operator>=(const BindLocation& location) const
    {
        return id >= location.id;
    }

    bool BindLocation::is_valid() const
    {
        return (*this) != undefined;
    }
}// namespace Engine
