#pragma once
#include <Core/object.hpp>
#include <Core/pointer.hpp>
#include <Graphics/shader_resource.hpp>
#include <Graphics/texture.hpp>

namespace Engine
{
    class ENGINE_EXPORT MaterialInterface : public Object
    {
        declare_class(MaterialInterface, Object);

    public:
    };


    class ENGINE_EXPORT Material : public MaterialInterface
    {
        declare_class(Material, MaterialInterface);

    public:
    };


    class ENGINE_EXPORT MaterialInstance : public MaterialInterface
    {
        declare_class(MaterialInstance, MaterialInterface);
    };

}// namespace Engine
