#pragma once
#include <Graphics/texture.hpp>
#include <unordered_map>


namespace Engine
{

    template<typename Type>
    using ResouceMap = std::unordered_map<ObjID, Type>;

    STRUCT Resources
    {
    public:
        ENGINE_EXPORT static const ResouceMap<Texture>& textures();
        ENGINE_EXPORT static const Resources& push_texture(const Texture& texture);
        ENGINE_EXPORT static const Resources& remove_texture(const Texture& texture);
    };

}// namespace Engine
