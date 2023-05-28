#pragma once
#include <Graphics/mesh_component.hpp>

namespace Engine::GLTF
{
    Vector<MeshComponent*> load_meshes(const String& path);
}
