#include <Core/buffer_manager.hpp>
#include <Core/class.hpp>
#include <Core/constants.hpp>
#include <Core/engine.hpp>
#include <Core/engine_config.hpp>
#include <Core/logger.hpp>
#include <Engine/ActorComponents/mesh_component.hpp>
#include <Graphics/material.hpp>
#include <numeric>
#include <strings.h>

namespace Engine
{

    implement_class(MeshComponent, "Engine", 0);
    implement_default_initialize_class(MeshComponent);
    implement_class(StaticMeshComponent, "Engine", 0);
    implement_default_initialize_class(StaticMeshComponent);
    implement_class(DynamicMeshComponent, "Engine", 0);
    implement_default_initialize_class(DynamicMeshComponent);
}// namespace Engine
