#include <iostream>
#include <model_loader.hpp>


namespace Engine
{
    const aiScene* load_scene(const std::string& filename)
    {
        Assimp::Importer importer;
        auto scene = importer.ReadFile(filename, aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_GenBoundingBoxes);
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            std::clog << "Model loader: " << importer.GetErrorString() << std::endl;
            return nullptr;
        }
        return scene;
    }
}// namespace Engine
