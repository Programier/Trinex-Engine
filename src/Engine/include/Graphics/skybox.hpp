#pragma once
#include <Graphics/texture_cubemap.hpp>
#include <Image/image.hpp>
#include <string>

namespace Engine
{
    class ENGINE_EXPORT Skybox : public TextureCubeMap
    {
    public:
        delete_copy_constructors(Skybox);
        Skybox();
        Skybox(const String& filename, const bool& invert = false);

        //! @brief {right, left, top, bottom, front, back}
        Skybox(const Vector<String>& filenames, const bool& invert = false);
        Skybox& load(const String& filename, const bool& invert = false);
        //! @brief {right, left, top, bottom, front, back}
        Skybox& load(const Vector<String>& filenames, const bool& invert = false);
        Skybox& draw();
    };

}// namespace Engine
