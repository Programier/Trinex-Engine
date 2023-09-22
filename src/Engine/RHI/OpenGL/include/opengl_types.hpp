#pragma once
#include <array>
#include <opengl_object.hpp>
#include <stdexcept>

namespace Engine
{

#define DECLARE_GETTER(b, array)                                                                                       \
    inline GLint get_type(const b& in_type)                                                                            \
    {                                                                                                                  \
        return array[static_cast<EnumerateType>(in_type)];                                                             \
    }

    template<typename ResultType, typename ElementType, std::size_t size>
    ResultType opengl_type_to_engine_type(const Array<ElementType, size>& containter, const ElementType& elem)
    {
        for (std::size_t i = 0; i < size; i++)
        {
            if (elem == containter[i])
                return static_cast<ResultType>(i);
        }

        throw std::runtime_error("OpenGL: Failed to convert OpenGL type to Engine type");
    }

#define DECLARE_TYPE(type, name, size)                                                                                 \
    extern const Array<GLint, size> _M_##name;                                                                    \
    DECLARE_GETTER(type, _M_##name)

    DECLARE_TYPE(TextureType, texture_types, 2);
    DECLARE_TYPE(CompareFunc, compare_funcs, 8);
    DECLARE_TYPE(CompareMode, compare_modes, 2);
    DECLARE_TYPE(SwizzleValue, swizzle_values, 7);
    DECLARE_TYPE(WrapValue, wrap_values, 5);
    DECLARE_TYPE(TextureCubeMapFace, cube_faces, 6);
    DECLARE_TYPE(IndexBufferComponent, index_components, 3);
    DECLARE_TYPE(StencilOp, stencil_options, 8);
    DECLARE_TYPE(PrimitiveTopology, primitive_topologies, 11);
    DECLARE_TYPE(BlendFunc, blend_funcs, 14);
    DECLARE_TYPE(BlendOp, blend_ops, 5);
    DECLARE_TYPE(CullMode, cull_modes, 4);


    struct ShaderType
    {
        byte size;
        GLenum type;
    };

    extern const Array<ShaderType, 19> _M_shader_types;

#undef DECLARE_TYPE
#undef DECLARE_GETTER
}// namespace Engine
