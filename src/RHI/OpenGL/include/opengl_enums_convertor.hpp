#pragma once
#include <Core/enums.hpp>
#include <opengl_headers.hpp>

namespace Engine
{
	GLuint compare_mode(CompareMode mode);
	GLuint compare_func(CompareFunc func);
	GLuint wrap_from(SamplerAddressMode value);
	GLuint blend_func(BlendFunc func, bool for_alpha);
	GLuint blend_op(BlendOp op);
	GLuint stencil_op(StencilOp op);
	GLuint convert_topology(PrimitiveTopology topology);
#if USING_OPENGL_CORE
	GLuint polygon_mode(PolygonMode mode);
#endif

	GLuint front_face(FrontFace face);
	GLuint cull_mode(CullMode mode);
}// namespace Engine
