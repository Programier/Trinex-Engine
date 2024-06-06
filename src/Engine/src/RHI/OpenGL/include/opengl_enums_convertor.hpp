#pragma once
#include <Core/enums.hpp>
#include <opengl_headers.hpp>

namespace Engine
{
    GLuint compare_mode(CompareMode mode);
    GLuint compare_func(CompareFunc func);
    GLuint wrap_from(WrapValue value);
    GLuint blend_func(BlendFunc func);
    GLuint blend_op(BlendOp op);
    GLuint depth_func(DepthFunc func);
    GLuint stencil_op(StencilOp op);
    GLuint convert_topology(PrimitiveTopology topology);
#if USING_OPENGL_CORE
    GLuint polygon_mode(PolygonMode mode);
    GLuint logic_op(LogicOp op);
#endif

    GLuint front_face(FrontFace face);
    GLuint cull_mode(CullMode mode);
}// namespace Engine
