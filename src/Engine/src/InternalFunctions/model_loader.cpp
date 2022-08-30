#include <LibLoader/lib_loader.hpp>
#include <iostream>
#include <model_loader.hpp>


namespace Engine
{
    const aiScene* load_scene(const std::string& filename)
    {
        Library assimp = load_library("assimp");
        if (!assimp.has_lib())
            return nullptr;

        auto assimp_ImportFile = assimp.get<const C_STRUCT aiScene*, const char*, unsigned int>(lib_function(aiImportFile));

        auto assimp_GetErrorString = assimp.get<const char*>(lib_function(aiGetErrorString));

        auto scene = assimp_ImportFile(filename.c_str(),
                                       aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_GenBoundingBoxes);
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            std::clog << "Model loader: " << assimp_GetErrorString() << std::endl;
            return nullptr;
        }
        return scene;
    }
}// namespace Engine
