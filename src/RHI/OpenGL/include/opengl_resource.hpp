#pragma once
#include <Graphics/rhi.hpp>

namespace Engine
{
	struct OpenGL_SRV : public RHI_DefaultDestroyable<RHI_ShaderResourceView> {
	};

	struct OpenGL_UAV : public RHI_DefaultDestroyable<RHI_UnorderedAccessView> {
	};

	struct OpenGL_RTV : public RHI_DefaultDestroyable<RHI_RenderTargetView> {
	};

	struct OpenGL_DSV : public RHI_DefaultDestroyable<RHI_DepthStencilView> {
	};
}// namespace Engine
