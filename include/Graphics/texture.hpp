#pragma once
#include <Core/asset.hpp>
#include <Core/math/vector.hpp>
#include <RHI/enums.hpp>
#include <RHI/resource_ptr.hpp>

namespace Trinex
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

	struct ENGINE_EXPORT Texture3DMip {
		Vector3u size;
		Buffer data;

		Texture3DMip(Vector3u init_size = {}, const Buffer& buffer = {}) : size(init_size), data(buffer) {}
		bool serialize(Archive& ar);
	};

	struct ENGINE_EXPORT TextureCubeMip {
		Vector2u size;
		Buffer data;

		TextureCubeMip(Vector2u init_size = {}, const Buffer& buffer = {}) : size(init_size), data(buffer) {}
		bool serialize(Archive& ar);
	};


	class ENGINE_EXPORT Texture : public Asset
	{
		trinex_class(Texture, Asset);

	protected:
		RHIResourcePtr<RHITexture> m_texture;

	public:
		RHIShaderResourceView* srv() const;
		RHITexture* handle() const;
	};

	class ENGINE_EXPORT Texture2D : public Texture
	{
		trinex_class(Texture2D, Texture);

	public:
		Vector<Texture2DMip> mips;
		RHIColorFormat format;

		Texture2D& rebuild() override;
		u32 width(u8 mip = 0) const;
		u32 height(u8 mip = 0) const;
		Vector2u size(u8 mip = 0) const;
		bool serialize(Archive& archive) override;
	};

	class ENGINE_EXPORT Texture3D : public Texture
	{
		trinex_class(Texture2D, Texture);

	public:
		Vector<Texture3DMip> mips;
		RHIColorFormat format;

		Texture3D& rebuild() override;
		u32 width(u8 mip = 0) const;
		u32 height(u8 mip = 0) const;
		u32 depth(u8 mip = 0) const;
		Vector3u size(u8 mip = 0) const;
		bool serialize(Archive& archive) override;
	};

	class ENGINE_EXPORT TextureCube : public Texture
	{
		trinex_class(TextureCube, Texture);

	public:
		Vector<TextureCubeMip> mips;
		RHIColorFormat format;

		TextureCube& rebuild() override;
		u32 width(u8 mip = 0) const;
		u32 height(u8 mip = 0) const;
		Vector2u size(u8 mip = 0) const;
		bool serialize(Archive& archive) override;
	};
}// namespace Trinex
