#include <Core/archive.hpp>
#include <Core/reflection/class.hpp>
#include <Core/reflection/property.hpp>
#include <Core/threading.hpp>
#include <Graphics/render_pools.hpp>
#include <Graphics/sampler.hpp>
#include <Graphics/texture.hpp>
#include <RHI/context.hpp>
#include <RHI/initializers.hpp>
#include <RHI/rhi.hpp>

namespace Trinex
{
	bool Texture2DMip::serialize(Archive& ar)
	{
		ar.serialize(size);
		ar.serialize_memory(data);
		return ar;
	}

	bool Texture3DMip::serialize(Archive& ar)
	{
		ar.serialize(size);
		ar.serialize_memory(data);
		return ar;
	}

	bool TextureCubeMip::serialize(Archive& ar)
	{
		ar.serialize(size);
		ar.serialize_memory(data);
		return ar;
	}

	trinex_implement_engine_class(Texture, Refl::Class::IsAsset) {}

	RHIShaderResourceView* Texture::rhi_srv() const
	{
		if (m_texture)
			return m_texture->as_srv();
		return nullptr;
	}

	RHITexture* Texture::rhi_texture() const
	{
		return m_texture.get();
	}

	trinex_implement_engine_class(Texture2D, Refl::Class::IsAsset)
	{
#define m_format format
		trinex_refl_prop(m_format, Refl::Property::IsReadOnly)->tooltip("Color format of this texture");
#undef m_format
	}

	Texture2D& Texture2D::init_render_resources()
	{
		RHITextureDesc desc = {
		        .type   = RHITextureType::Texture2D,
		        .format = format,
		        .size   = {size(), 1},
		        .mips   = static_cast<u32>(mips.size()),
		        .flags  = RHITextureFlags::ShaderResource,
		};

		m_texture = RHI::instance()->create_texture(desc);

		RHIContextPool::global_instance()->execute([this](RHIContext* ctx) {
			ctx->barrier(m_texture, RHIAccess::TransferDst);

			for (u8 index = 0; auto& mip : mips)
			{
				RHITextureRegion region(mip.size, {0, 0}, index++);
				ctx->update(m_texture, region, mip.data.data(), {.size = mip.data.size()});
			}

			ctx->barrier(m_texture, RHIAccess::SRVGraphics);
		});

		return *this;
	}

	u32 Texture2D::width(u8 mip) const
	{
		return mips.size() <= static_cast<usize>(mip) ? mips[mip].size.x : 0;
	}

	u32 Texture2D::height(u8 mip) const
	{
		return mips.size() <= static_cast<usize>(mip) ? mips[mip].size.y : 0;
	}

	Vector2u Texture2D::size(u8 mip) const
	{
		return mips.size() <= static_cast<usize>(mip) ? Vector2u{0, 0} : mips[mip].size;
	}

	bool Texture2D::serialize(Archive& archive)
	{
		if (!Super::serialize(archive))
			return false;
		return archive.serialize(mips);
	}

	trinex_implement_engine_class(Texture3D, Refl::Class::IsAsset)
	{
		trinex_refl_prop(format, Refl::Property::IsReadOnly)->tooltip("Color format of this texture");
	}

	Texture3D& Texture3D::init_render_resources()
	{
		RHITextureDesc desc = {
		        .type   = RHITextureType::Texture3D,
		        .format = format,
		        .size   = size(),
		        .mips   = static_cast<u32>(mips.size()),
		        .flags  = RHITextureFlags::ShaderResource,
		};

		m_texture = RHI::instance()->create_texture(desc);

		RHIContextPool::global_instance()->execute([this](RHIContext* ctx) {
			ctx->barrier(m_texture, RHIAccess::TransferDst);

			for (u8 index = 0; auto& mip : mips)
			{
				RHITextureRegion region(mip.size, {0, 0, 0}, index++);
				ctx->update(m_texture, region, mip.data.data(), {.size = mip.data.size()});
			}

			ctx->barrier(m_texture, RHIAccess::SRVGraphics);
		});

		return *this;
	}

	u32 Texture3D::width(u8 mip) const
	{
		return mips.size() <= static_cast<usize>(mip) ? mips[mip].size.x : 0;
	}

	u32 Texture3D::height(u8 mip) const
	{
		return mips.size() <= static_cast<usize>(mip) ? mips[mip].size.y : 0;
	}

	u32 Texture3D::depth(u8 mip) const
	{
		return mips.size() <= static_cast<usize>(mip) ? mips[mip].size.z : 0;
	}

	Vector3u Texture3D::size(u8 mip) const
	{
		return mips.size() <= static_cast<usize>(mip) ? Vector3u{0, 0, 0} : mips[mip].size;
	}

	bool Texture3D::serialize(Archive& archive)
	{
		if (!Super::serialize(archive))
			return false;
		return archive.serialize(mips);
	}

	trinex_implement_engine_class(TextureCube, Refl::Class::IsAsset)
	{
		trinex_refl_prop(format, Refl::Property::IsReadOnly)->tooltip("Color format of this texture");
	}

	TextureCube& TextureCube::init_render_resources()
	{
		RHITextureDesc desc = {
		        .type   = RHITextureType::TextureCube,
		        .format = format,
		        .size   = {size(), 1},
		        .mips   = static_cast<u32>(mips.size()),
		        .flags  = RHITextureFlags::ShaderResource,
		};

		m_texture = RHI::instance()->create_texture(desc);

		RHIContextPool::global_instance()->execute([this](RHIContext* ctx) {
			ctx->barrier(m_texture, RHIAccess::TransferDst);

			for (u8 index = 0; auto& mip : mips)
			{
				const u8* data       = mip.data.data();
				const usize mip_size = mip.data.size() / 6;

				for (u8 face = 0; face < 6; ++face)
				{
					RHITextureRegion region(mip.size, {0, 0}, index);
					region.slice = face;
					ctx->update(m_texture, region, data, {.size = mip_size});
					data += mip_size;
				}

				++index;
			}

			ctx->barrier(m_texture, RHIAccess::SRVGraphics);
		});

		return *this;
	}// namespace Trinex

	u32 TextureCube::width(u8 mip) const
	{
		return mips.size() <= static_cast<usize>(mip) ? mips[mip].size.x : 0;
	}

	u32 TextureCube::height(u8 mip) const
	{
		return mips.size() <= static_cast<usize>(mip) ? mips[mip].size.y : 0;
	}

	Vector2u TextureCube::size(u8 mip) const
	{
		return mips.size() <= static_cast<usize>(mip) ? Vector2u{0, 0} : mips[mip].size;
	}

	bool TextureCube::serialize(Archive& archive)
	{
		if (!Super::serialize(archive))
			return false;
		return archive.serialize(mips);
	}
}// namespace Trinex
