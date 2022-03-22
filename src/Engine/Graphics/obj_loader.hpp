#pragma once

#include "mesh.hpp"
#include "texturearray.hpp"

namespace Engine
{
    void load_obj(TextureArray& texture, Mesh& mesh, const std::string& filename,
                  bool invert_vertical = true);
}
