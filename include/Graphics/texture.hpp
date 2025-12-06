#pragma once
#include <Core/math/vector.hpp>
#include <Graphics/render_resource.hpp>
#include <RHI/enums.hpp>
#include <RHI/resource_ptr.hpp>

namespace Engine
{
	class RHIShaderResourceView;
	class RHISampler;
	class RHITexture;
	class Sampler;

	struct ENGINE_EXPORT Texture2DMip {
		Vector2u size;
		Buffer data;

		Texture2DMip(Vector2u init_size = {}, const Buffer& buffer = {}) : size(init_size), data(buffer) {}
		bool serialize(Archive& ar);
	};

	struct ENGINE_EXPORT TextureCubeMip {
		Vector2u size;
		Buffer data;

		TextureCubeMip(Vector2u init_size = {}, const Buffer& buffer = {}) : size(init_size), data(buffer) {}
		bool serialize(Archive& ar);
	};

	class ENGINE_EXPORT Texture : public RenderResource
	{
		trinex_class(Texture, RenderResource);

	protected:
		RHIResourcePtr<RHITexture> m_texture;

	public:
		RHIShaderResourceView* rhi_srv() const;
		RHITexture* rhi_texture() const;
	};

	class ENGINE_EXPORT Texture2D : public Texture
	{
		trinex_class(Texture2D, Texture);

	public:
		Vector<Texture2DMip> mips;
		RHIColorFormat format;

		Texture2D& init_render_resources() override;
		uint_t width(byte mip = 0) const;
		uint_t height(byte mip = 0) const;
		Vector2u size(byte mip = 0) const;
		bool serialize(Archive& archive) override;
	};

	class ENGINE_EXPORT TextureCube : public Texture
	{
		trinex_class(TextureCube, Texture);

	public:
		Vector<TextureCubeMip> mips;
		RHIColorFormat format;

		TextureCube& init_render_resources() override;
		uint_t width(byte mip = 0) const;
		uint_t height(byte mip = 0) const;
		Vector2u size(byte mip = 0) const;
		bool serialize(Archive& archive) override;
	};
}// namespace Engine
