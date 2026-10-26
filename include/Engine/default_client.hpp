#pragma once
#include <Core/etl/array.hpp>
#include <Core/viewport_client.hpp>
#include <RHI/types.hpp>

namespace Trinex
{
	class World;
	class RHIObject;
	class RHIBuffer;
	class RHIContext;

	// This viewport can be used for testing something
	class trinex_api DefaultClient : public ViewportClient
	{
		trinex_class(DefaultClient, ViewportClient);

	private:
		World* m_world;
		RHIBuffer* m_scene;
		Array<RHIObject*> m_resources;

	private:
		RHIBuffer* create_buffer(RHIContext* ctx, const void* data, usize size);
		RHIBuffer* create_buffer(RHIContext* ctx, usize size);
		RHIDescriptor create_descriptor(RHIContext* ctx, const void* data, usize size);

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
		DefaultClient& attach(class RenderViewport* viewport) override;
		DefaultClient& update(class RenderViewport* viewport, float dt) override;
	};
} // namespace Trinex
