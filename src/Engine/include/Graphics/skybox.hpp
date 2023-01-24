#pragma once
#include <Graphics/texture_cubemap.hpp>
#include <Image/image.hpp>
#include <TemplateFunctional/array_containers.hpp>
#include <string>

namespace Engine
{
    CLASS Skybox : public TextureCubeMap
    {
        declare_instance_info_hpp(Skybox);

    public:
        delete_copy_constructors(Skybox);
        constructor_hpp(Skybox);
        Skybox(const std::string& filename, const bool& invert = false);

        //! @brief {right, left, top, bottom, front, back}
        Skybox(const DynamicArray<std::string>& filenames, const bool& invert = false);
        Skybox& load(const std::string& filename, const bool& invert = false);
        //! @brief {right, left, top, bottom, front, back}
        Skybox& load(const DynamicArray<std::string>& filenames, const bool& invert = false);
        Skybox& draw();
    };

}// namespace Engine
