#include <Core/archive.hpp>
#include <Core/base_engine.hpp>
#include <Core/buffer_manager.hpp>
#include <Core/logger.hpp>
#include <Core/reflection/class.hpp>
#include <Core/reflection/enum.hpp>
#include <Core/reflection/property.hpp>
#include <Graphics/rhi.hpp>
#include <Graphics/texture_2D.hpp>
#include <Image/image.hpp>

namespace Engine
{
	bool Texture2DMip::serialize(Archive& ar)
	{
		return ar.serialize(size, data);
	}

	void (*Texture2D::generate_mips)(ColorFormat format, Vector<Texture2DMip>&) = nullptr;

	trinex_implement_engine_class(Texture2D, Refl::Class::IsAsset)
	{
		auto* self = static_class_instance();

		trinex_refl_prop(self, This, path)->tooltip("Path to texture");
		trinex_refl_prop(self, This, m_format, Refl::Property::IsReadOnly)->tooltip("Color format of this texture");
	}

	Texture2D& Texture2D::init(ColorFormat format, Size2D size, const Buffer& data, bool need_generate_mips)
	{
		return init(format, size, data.data(), data.size(), need_generate_mips);
	}

	Texture2D& Texture2D::init(ColorFormat format, Size2D size, const byte* data, size_t data_size, bool need_generate_mips)
	{
		if (data == nullptr || data_size == 0)
			return init(format, size);

		m_format = format;

		m_mips.clear();
		m_mips.emplace_back(size, Buffer(data, data + data_size));

		if (need_generate_mips && generate_mips)
		{
			generate_mips(format, m_mips);
		}

		init_resource();
		return *this;
	}

	Texture2D& Texture2D::init(const Image& image, bool need_generate_mips)
	{
		return init(image.format(), image.size(), image.buffer(), need_generate_mips);
	}

	Texture2D& Texture2D::init(ColorFormat format, Size2D size)
	{
		m_format = format;

		m_mips.clear();
		m_mips.emplace_back(size, Buffer{});
		init_resource();
		return *this;
	}

	MipMapLevel Texture2D::mipmap_count() const
	{
		return m_mips.size();
	}

	float Texture2D::width(MipMapLevel mip) const
	{
		if (static_cast<MipMapLevel>(m_mips.size()) <= mip)
			return 0;
		return m_mips[mip].size.x;
	}

	float Texture2D::height(MipMapLevel mip) const
	{
		if (static_cast<MipMapLevel>(m_mips.size()) <= mip)
			return 0;
		return m_mips[mip].size.y;
	}

	Size2D Texture2D::size(MipMapLevel mip) const
	{
		if (static_cast<MipMapLevel>(m_mips.size()) <= mip)
			return {0.f, 0.f};
		return m_mips[mip].size;
	}

	ColorFormat Texture2D::format() const
	{
		return m_format;
	}

	const Texture2DMip* Texture2D::mip(MipMapLevel level) const
	{
		if (static_cast<MipMapLevel>(m_mips.size()) <= level)
			return nullptr;
		return &m_mips[level];
	}

	Texture2D& Texture2D::generate_mipmaps()
	{
		if (m_mips.empty() || generate_mips == nullptr)
			return *this;

		Texture2DMip mip = std::move(m_mips[0]);
		m_mips.clear();
		m_mips.emplace_back(std::move(mip));

		generate_mips(m_format, m_mips);
		return *this;
	}

	Texture2D& Texture2D::rhi_init()
	{
		m_rhi_object.reset(rhi->create_texture_2d(this));
		return *this;
	}

	bool Texture2D::serialize(Archive& archive)
	{
		if (!Texture::serialize(archive))
		{
			return false;
		}

		archive.serialize(m_mips);
		return static_cast<bool>(archive);
	}

	TextureType Texture2D::type() const
	{
		return TextureType::Texture2D;
	}

	Texture2D& Texture2D::apply_changes()
	{
		Super::apply_changes();
		Image image(path);

		if (!image.empty())
		{
			m_mips.clear();
			m_mips.emplace_back();
			auto& mip = m_mips.back();
			mip.size  = image.size();
			mip.data  = image.buffer();
			m_format  = image.format();

			init_resource();
		}
		return *this;
	}
}// namespace Engine
