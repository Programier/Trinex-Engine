#pragma once
#include <BasicFunctional/dynamic_array.hpp>
#include <Graphics/texture.hpp>
#include <Image/image.hpp>
#include <string>

namespace Engine
{
    enum class SkyboxType
    {
        CubeMap,
        Cylindric
    };

    class Skybox
    {
        ObjectID _M_ID = 0;
        Image _M_images[6];
        SkyboxType _M_type = SkyboxType::CubeMap;
        Texture _M_cylindric_texture;
        void update_id();

        void delete_skybox();

    public:
        Skybox();
        Skybox(const std::string& filename, const SkyboxType& type = SkyboxType::CubeMap, const bool& invert = false);

        //! @brief {right, left, top, bottom, front, back}
        Skybox(const DynamicArray<std::string>& filenames, const bool& invert = false);
        Skybox(const Skybox&);

        Skybox& operator=(const Skybox&);


        Skybox& load(const std::string& filename, const SkyboxType& type = SkyboxType::CubeMap, const bool& invert = false);
        //! @brief {right, left, top, bottom, front, back}
        Skybox& load(const DynamicArray<std::string>& filenames, const bool& invert = false);
        Skybox& draw();
        SkyboxType type() const;
        ~Skybox();
    };

}// namespace Engine
