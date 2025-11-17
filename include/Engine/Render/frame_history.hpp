#pragma once

namespace Engine
{
	class RHITexture;
	class RHIBuffer;
	
	struct ENGINE_EXPORT FrameHistory {
		RHITexture* scene   = nullptr;
		RHITexture* normal  = nullptr;
		RHITexture* depth   = nullptr;
		RHIBuffer* exposure = nullptr;
	};
}// namespace Engine
