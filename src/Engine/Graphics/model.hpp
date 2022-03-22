#pragma once

#include "mesh.hpp"
#include "texturearray.hpp"
#include <list>



namespace Engine
{
    struct model
    {
        TextureArray _M_array;
        Mesh _M_mesh;
    };

    class Model
    {
       std::list<model> _M_models;

    public:
        Model();
    };
}// namespace Engine
