#include <Core/names.hpp>


namespace Engine::Names
{
#define declare_name(name) const Name name = #name
    declare_name(model);
    declare_name(texture);
}// namespace Engine::Names
