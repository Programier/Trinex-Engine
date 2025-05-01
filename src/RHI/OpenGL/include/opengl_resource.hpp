#pragma once
#include <Graphics/rhi.hpp>

namespace Engine
{
	struct OpenGL_Sampler;

	struct OpenGL_SRV : public RHI_ShaderResourceView {
		virtual void bind(byte location, OpenGL_Sampler* sampler) = 0;
	};

	struct OpenGL_UAV : public RHI_UnorderedAccessView {
		virtual void bind(byte location) = 0;
	};

	struct OpenGL_RTV : public RHI_RenderTargetView {
	};

	struct OpenGL_DSV : public RHI_DepthStencilView {
	};
}// namespace Engine
