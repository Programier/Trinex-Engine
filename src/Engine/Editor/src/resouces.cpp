#include <resouces.hpp>


namespace Editor
{
    Engine::Scene Resources::scene;
    Engine::ObjectInstance* Resources::object_for_rendering = &Resources::scene;
    Engine::ObjectInstance* Resources::object_for_properties = &Resources::scene;
}// namespace Editor
