#pragma once
#include <Graphics/mesh.hpp>

namespace Engine::GLTF
{
    Vector<Mesh*> load_meshes(const String& path);
}
