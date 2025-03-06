#pragma once
#include <Core/filesystem/path.hpp>
#include <Graphics/texture.hpp>
#include <Image/image.hpp>

namespace Engine
{
	struct ENGINE_EXPORT Texture2DMip {
		Size2D size;
		Buffer data;

		Texture2DMip(Size2D init_size = {}, const Buffer& buffer = {}) : size(glm::abs(init_size)), data(buffer)
		{}

		bool serialize(Archive& ar);
	};

	class ENGINE_EXPORT Texture2D : public Texture
	{
		trinex_declare_class(Texture2D, Texture);

		Vector<Texture2DMip> m_mips;
		ColorFormat m_format;

	public:
		Path path;
		static void (*generate_mips)(ColorFormat format, Vector<Texture2DMip>&);

		Texture2D& init(ColorFormat format, Size2D size, const Buffer& data, bool generate_mips = false);
		Texture2D& init(ColorFormat format, Size2D size, const byte* data, size_t data_size, bool generate_mips = false);
		Texture2D& init(const Image& image, bool generate_mips = false);
		Texture2D& init(ColorFormat format, Size2D size);

		// Information block
		MipMapLevel mipmap_count() const;
		float width(MipMapLevel mip = 0) const;
		float height(MipMapLevel mip = 0) const;
		Size2D size(MipMapLevel mip = 0) const;
		ColorFormat format() const;
		const Texture2DMip* mip(MipMapLevel level = 0) const;

		Texture2D& generate_mipmaps();
		Texture2D& rhi_init() override;
		TextureType type() const override;
		Texture2D& apply_changes() override;

		bool serialize(Archive& archive) override;
	};
}// namespace Engine
