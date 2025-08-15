#include <Core/archive.hpp>
#include <Core/reflection/class.hpp>
#include <Core/reflection/property.hpp>
#include <Core/threading.hpp>
#include <Graphics/sampler.hpp>
#include <Graphics/texture.hpp>
#include <RHI/rhi.hpp>

namespace Engine
{
	bool Texture2DMip::serialize(Archive& ar)
	{
		ar.serialize(size, data);
		return true;
	}

	bool TextureCubeMip::serialize(Archive& ar)
	{
		return ar.serialize(size, data);
	}

	trinex_implement_engine_class(Texture, Refl::Class::IsAsset) {}

	Texture& Texture::rhi_bind(byte location)
	{
		rhi->bind_srv(rhi_srv(), location);
		return *this;
	}

	Texture& Texture::rhi_bind_combined(byte location, RHISampler* sampler)
	{
		rhi->bind_srv(rhi_srv(), location);
		rhi->bind_sampler(sampler, location);
		return *this;
	}

	Texture& Texture::rhi_bind_combined(byte location, Sampler* sampler)
	{
		rhi->bind_srv(rhi_srv(), location);
		rhi->bind_sampler(sampler->rhi_sampler(), location);
		return *this;
	}

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
		render_thread()->call([this]() {
			m_texture = rhi->create_texture(RHITextureType::Texture2D, format, {size(), 1}, mips.size(),
			                                RHITextureCreateFlags::ShaderResource);

			for (byte index = 0; auto& mip : mips)
			{
				RHITextureRegion region(mip.size, {0, 0}, index++);
				rhi->update_texture(m_texture, region, mip.data.data(), mip.data.size());
			}
		});
		return *this;
	}

	uint_t Texture2D::width(byte mip) const
	{
		return mips.size() <= static_cast<size_t>(mip) ? mips[mip].size.x : 0;
	}

	uint_t Texture2D::height(byte mip) const
	{
		return mips.size() <= static_cast<size_t>(mip) ? mips[mip].size.y : 0;
	}

	Vector2u Texture2D::size(byte mip) const
	{
		return mips.size() <= static_cast<size_t>(mip) ? Vector2u{0, 0} : mips[mip].size;
	}

	bool Texture2D::serialize(Archive& archive)
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
		render_thread()->call([this]() {
			m_texture = rhi->create_texture(RHITextureType::TextureCube, format, {size(), 1}, mips.size(),
			                                RHITextureCreateFlags::ShaderResource);

			for (byte index = 0; auto& mip : mips)
			{
				const byte* data      = mip.data.data();
				const size_t mip_size = mip.data.size() / 6;

				for (byte face = 0; face < 6; ++face)
				{
					RHITextureRegion region(mip.size, {0, 0}, index);
					region.array_slice = face;
					rhi->update_texture(m_texture, region, data, mip_size);
					data += mip_size;
				}

				++index;
			}
		});
		return *this;
	}

	uint_t TextureCube::width(byte mip) const
	{
		return mips.size() <= static_cast<size_t>(mip) ? mips[mip].size.x : 0;
	}

	uint_t TextureCube::height(byte mip) const
	{
		return mips.size() <= static_cast<size_t>(mip) ? mips[mip].size.y : 0;
	}

	Vector2u TextureCube::size(byte mip) const
	{
		return mips.size() <= static_cast<size_t>(mip) ? Vector2u{0, 0} : mips[mip].size;
	}

	bool TextureCube::serialize(Archive& archive)
	{
		if (!Super::serialize(archive))
			return false;
		return archive.serialize(mips);
	}
}// namespace Engine
