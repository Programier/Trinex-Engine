#pragma once
#include <Core/filesystem/path.hpp>
#include <Core/math/vector.hpp>
#include <Graphics/texture.hpp>
#include <Image/image.hpp>
#include <RHI/enums.hpp>

namespace Engine
{
	struct ENGINE_EXPORT Texture2DMip {
		Vector2u size;
		Buffer data;

		Texture2DMip(Vector2u init_size = {}, const Buffer& buffer = {}) : size(init_size), data(buffer) {}
		bool serialize(Archive& ar);
	};

	class ENGINE_EXPORT Texture2D : public Texture
	{
		trinex_declare_class(Texture2D, Texture);

	public:
		Vector<Texture2DMip> mips;
		RHIColorFormat format;

		Texture2D& init_render_resources() override;
		uint_t width(byte mip = 0) const;
		uint_t height(byte mip = 0) const;
		Vector2u size(byte mip = 0) const;
		bool serialize(Archive& archive) override;
	};
}// namespace Engine
