#include <Engine/scene_view_state.hpp>
#include <Graphics/render_pools.hpp>
#include <RHI/context.hpp>
#include <RHI/handles.hpp>

namespace Trinex
{
	static void release_resource(RHIBuffer*& buffer)
	{
		if (buffer)
		{
			RHIBufferPool::global_instance()->return_buffer(buffer);
			buffer = nullptr;
		}
	}

	static void release_resource(RHITexture*& texture)
	{
		if (texture)
		{
			RHITexturePool::global_instance()->return_surface(texture);
			texture = nullptr;
		}
	}

	SceneViewState& SceneViewState::release()
	{
		release_resource(m_scene_color);
		return *this;
	}

	SceneViewState& SceneViewState::allocate(RHIContext* ctx, Vector2u size)
	{
		m_size        = size;
		m_scene_color = RHITexturePool::global_instance()->request_surface(RHISurfaceFormat::RGBA16F, size,
		                                                                   RHITextureFlags::RWColorAttachment);

		ctx->barrier(m_scene_color, RHIAccess::TransferDst);
		ctx->clear_rtv(m_scene_color->as_rtv());
		return *this;
	}

	SceneViewState::~SceneViewState()
	{
		release();
	}

	SceneViewState& SceneViewState::resize(RHIContext* ctx, Vector2u size)
	{
		if (m_size != size)
		{
			release().allocate(ctx, size);
		}
		return *this;
	}

	SceneViewState& SceneViewState::store(const SceneView& view)
	{
		return *this;
	}
}// namespace Trinex
