#pragma once
#include <Graphics/rhi.hpp>
#include <d3d11.h>

namespace Engine
{
	class D3D11_Texture2D : public RHI_DefaultDestroyable<RHI_Texture>
	{
	public:
		ID3D11Texture2D* m_texture = nullptr;

		bool init(const Texture2D* texture);

		static RHI_ShaderResourceView* static_create_srv(RHI_Object* owner, ID3D11Texture2D* texture);
		static RHI_UnorderedAccessView* static_create_uav(RHI_Object* owner, ID3D11Texture2D* texture);

		RHI_ShaderResourceView* create_srv() override;
		RHI_UnorderedAccessView* create_uav() override;
		~D3D11_Texture2D();
	};

	class D3D11_TextureSRV : public RHI_DefaultDestroyable<RHI_ShaderResourceView>
	{
	public:
		RHI_Object* m_owner;
		ID3D11ShaderResourceView* m_view;

		D3D11_TextureSRV(RHI_Object* owner, ID3D11ShaderResourceView* view);
		void bind(BindLocation location) override;
		void bind_combined(byte location, struct RHI_Sampler* sampler) override;
		~D3D11_TextureSRV();
	};

	class D3D11_TextureUAV : public RHI_DefaultDestroyable<RHI_UnorderedAccessView>
	{
	public:
		RHI_Object* m_owner;
		ID3D11UnorderedAccessView* m_view;

		D3D11_TextureUAV(RHI_Object* owner, ID3D11UnorderedAccessView* view);
		void bind(BindLocation location) override;
		~D3D11_TextureUAV();
	};
}// namespace Engine
