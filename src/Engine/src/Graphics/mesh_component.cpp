#include <Core/buffer_manager.hpp>
#include <Core/class.hpp>
#include <Core/constants.hpp>
#include <Core/engine.hpp>
#include <Core/engine_config.hpp>
#include <Core/logger.hpp>
#include <Graphics/material.hpp>
#include <Graphics/mesh_component.hpp>
#include <numeric>
#include <strings.h>

namespace Engine
{

    implement_class(MeshComponent, "Engine");
    implement_default_initialize_class(MeshComponent);
    implement_class(StaticMeshComponent, "Engine");
    implement_default_initialize_class(StaticMeshComponent);
    implement_class(DynamicMeshComponent, "Engine");
    implement_default_initialize_class(DynamicMeshComponent);
}// namespace Engine
