#include <Engine/Render/frame_history.hpp>
#include <Graphics/render_pools.hpp>

namespace Engine
{
	void FrameHistory::release_texture(RHITexture* texture)
	{
		RHITexturePool::global_instance()->return_surface(texture);
	}

	void FrameHistory::release_buffer(RHIBuffer* buffer)
	{
		RHIBufferPool::global_instance()->return_buffer(buffer);
	}
}// namespace Engine
