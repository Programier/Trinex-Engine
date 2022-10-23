#pragma once
#include <string>
#include <TemplateFunctional/dynamic_array.hpp>
#include <Graphics/texture_cubemap.hpp>
#include <Image/image.hpp>

namespace Engine
{
    CLASS Skybox : public TextureCubeMap
    {
    public:
        implement_class_hpp(Skybox);
        Skybox(const std::string& filename, const bool& invert = false);

        //! @brief {right, left, top, bottom, front, back}
        Skybox(const DynamicArray<std::string>& filenames, const bool& invert = false);
        Skybox& load(const std::string& filename, const bool& invert = false);
        //! @brief {right, left, top, bottom, front, back}
        Skybox& load(const DynamicArray<std::string>& filenames, const bool& invert = false);
        Skybox& draw();
    };

}// namespace Engine
