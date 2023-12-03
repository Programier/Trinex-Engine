#pragma once
#include <Engine/ActorComponents/mesh_component.hpp>

namespace Engine::GLTF
{
    Vector<MeshComponent*> load_meshes(const String& path);
}
