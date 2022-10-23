#pragma once
#include <Graphics/camera.hpp>

namespace Engine
{
    CLASS Scene
    {
    private:

        // Cameras parameters
        std::vector<Camera> cameras;
        Camera* main_camera;

        // Scene Textures


    public:
        Scene();
        Scene& load(const std::string& filename);


        Scene& render() const;

    };
}// namespace Engine
