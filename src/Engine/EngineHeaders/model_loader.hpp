#pragma once

#include <assimp/Importer.hpp>
#include <assimp/cimport.h>
#include <assimp/matrix4x4.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>


namespace Engine
{
    const aiScene* load_scene(const std::string& filename);
}
