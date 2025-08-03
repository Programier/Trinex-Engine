#pragma once
#include <Core/engine_types.hpp>
#include <Core/math/vector.hpp>

namespace Engine
{
	struct LinearColor;

	struct Color : VectorNT<4, byte> {
		using VectorNT<4, byte>::VectorNT;

		explicit Color(uint32_t rgba)
		{
			r = (rgba >> 24) & 0xFF;
			g = (rgba >> 16) & 0xFF;
			b = (rgba >> 8) & 0xFF;
			a = rgba & 0xFF;
		}

		explicit Color(const LinearColor& linear);
		Color& operator=(const LinearColor& linear);

		inline uint32_t as_uint() const { return (x << 24) | (y << 16) | (z << 8) | w; }
	};

	struct LinearColor : VectorNT<4, float_t> {
		using VectorNT<4, float_t>::VectorNT;

		explicit LinearColor(const Color& color)
		{
			x = static_cast<float>(color.x) / 255.f;
			y = static_cast<float>(color.y) / 255.f;
			z = static_cast<float>(color.z) / 255.f;
			w = static_cast<float>(color.w) / 255.f;
		}

		LinearColor& operator=(const Color& color)
		{
			x = static_cast<float>(color.x) / 255.f;
			y = static_cast<float>(color.y) / 255.f;
			z = static_cast<float>(color.z) / 255.f;
			w = static_cast<float>(color.w) / 255.f;
			return *this;
		}
	};

	inline Color& Color::operator=(const LinearColor& linear)
	{
		auto as_byte = [](float value) -> byte {
			value = value < 0.0f ? 0.0f : (value > 1.0f ? 1.0f : value);
			return static_cast<byte>(value * 255.0f + 0.5f);
		};

		x = as_byte(linear.x);
		y = as_byte(linear.y);
		z = as_byte(linear.z);
		w = as_byte(linear.w);
		return *this;
	}

	inline Color::Color(const LinearColor& linear)
	{
		(*this) = linear;
	}
}// namespace Engine
