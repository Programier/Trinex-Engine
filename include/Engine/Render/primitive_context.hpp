#pragma once
#include <Core/math/matrix.hpp>

namespace Engine
{
	class Renderer;
	class RHIContext;
	class RenderPass;
	class MaterialBindings;

	struct PrimitiveRenderingContext {
		class Renderer* renderer;
		RHIContext* context;
		RenderPass* render_pass;
		const Matrix4f* local_to_world;
		const MaterialBindings* bindings;

		inline PrimitiveRenderingContext(Renderer* renderer = nullptr, RHIContext* ctx = nullptr,
		                                 RenderPass* render_pass = nullptr, const Matrix4f* local_to_world = nullptr,
		                                 const MaterialBindings* bindings = nullptr)
		    : renderer(renderer), context(ctx), render_pass(render_pass), local_to_world(local_to_world), bindings(nullptr)
		{}
	};
}// namespace Engine
