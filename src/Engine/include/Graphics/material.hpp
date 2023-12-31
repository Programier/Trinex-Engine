#pragma once
#include <Core/object.hpp>
#include <Core/pointer.hpp>
#include <Core/structures.hpp>
#include <Graphics/texture.hpp>

namespace Engine
{
    class Pipeline;

    struct ENGINE_EXPORT VertexBufferStream {
        BindingIndex stream;
        VertexBufferSemantic semantic;
        byte index = 0;

        FORCE_INLINE bool operator==(const VertexBufferStream& other) const
        {
            return other.stream == stream && other.semantic == semantic && other.index == index;
        }

        FORCE_INLINE bool operator!=(const VertexBufferStream& other) const
        {
            return !((*this) == other);
        }
    };

    class ENGINE_EXPORT MaterialInterface : public Object
    {
        declare_class(MaterialInterface, Object);
    };

    class ENGINE_EXPORT Material : public MaterialInterface
    {
        declare_class(Material, MaterialInterface);

    public:
        Pointer<Pipeline> pipeline;

        Material();
    };

    class ENGINE_EXPORT MaterialInstance : public MaterialInterface
    {
        declare_class(MaterialInstance, MaterialInterface);
    };

    class ENGINE_EXPORT MaterialObject : public Object
    {
        declare_class(MaterialObject, Object);
    };

}// namespace Engine
