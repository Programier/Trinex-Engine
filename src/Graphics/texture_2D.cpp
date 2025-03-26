#include <Core/archive.hpp>
#include <Core/reflection/class.hpp>
#include <Core/reflection/property.hpp>
#include <Core/threading.hpp>
#include <Graphics/rhi.hpp>
#include <Graphics/texture_2D.hpp>

namespace Engine
{
	bool Texture2DMip::serialize(Archive& ar)
	{
		ar.serialize(size, data);
		return true;
	}

	trinex_implement_engine_class(Texture2D, Refl::Class::IsAsset)
	{
		auto* self = static_class_instance();
#define m_format format
		trinex_refl_prop(self, This, m_format, Refl::Property::IsReadOnly)->tooltip("Color format of this texture");
	}

	Texture2D& Texture2D::init_render_resources()
	{
		render_thread()->call([this]() {
			m_texture = rhi->create_texture_2d(m_format, size(), mips.size(), TextureCreateFlags::ShaderResource);
			m_srv     = m_texture->create_srv();

			for (byte index = 0; auto& mip : mips)
			{
				m_texture->update(index, Rect2D({0, 0}, mip.size), mip.data.data(), mip.data.size());
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

	TextureType Texture2D::type() const
	{
		return TextureType::Texture2D;
	}

	bool Texture2D::serialize(Archive& archive)
	{
		if (!Super::serialize(archive))
			return false;
		archive.serialize(mips);
		return true;
	}
}// namespace Engine
