#pragma once
#include <Core/etl/vector.hpp>
#include <Graphics/render_viewport.hpp>

namespace Trinex
{
	class World;
	class RHIObject;
	class RHIBuffer;
	class RHIContext;

	// This viewport can be used for testing something
	class ENGINE_EXPORT DefaultClient : public ViewportClient
	{
		trinex_class(DefaultClient, ViewportClient);

	private:
		World* m_world;
		RHIBuffer* m_scene;
		Vector<RHIObject*> m_resources;

	private:
		RHIBuffer* create_buffer(RHIContext* ctx, const void* data, usize size);

		template<typename T>
		inline RHIBuffer* create_buffer(RHIContext* ctx, std::initializer_list<T> data)
		{
			return create_buffer(ctx, data.begin(), data.size() * sizeof(T));
		}

		template<typename T>
		inline RHIBuffer* create_buffer(RHIContext* ctx, const T& data)
		{
			return create_buffer(ctx, &data, sizeof(T));
		}

	public:
		DefaultClient();
		~DefaultClient();
		DefaultClient& on_bind_viewport(class RenderViewport* viewport) override;
		DefaultClient& update(class RenderViewport* viewport, float dt) override;
	};
}// namespace Trinex
