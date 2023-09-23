#include <Core/class.hpp>
#include <Core/logger.hpp>
#include <Core/string_functions.hpp>
#include <Graphics/material.hpp>
#include <Graphics/mesh_component.hpp>
#include <Graphics/shader.hpp>
#include <Graphics/texture.hpp>


namespace Engine
{

    implement_class(MaterialInterface, "Engine");
    implement_class(Material, "Engine");
    implement_class(MaterialInstance, "Engine");
    implement_default_initialize_class(MaterialInterface);
    implement_default_initialize_class(Material);
    implement_default_initialize_class(MaterialInstance);
}// namespace Engine
