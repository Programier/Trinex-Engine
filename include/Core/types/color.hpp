#pragma once
#include <Core/engine_types.hpp>
#include <Core/math/vector.hpp>

namespace Engine
{
	struct LinearColor;

	struct Color {
		union
		{
			struct {
				byte r, g, b, a;
			};

			uint32_t rgba;
		};

		constexpr Color() : r(0), g(0), b(0), a(255) {}
		constexpr Color(byte r, byte g, byte b, byte a = 255) : r(r), g(g), b(b), a(a) {}
		constexpr Color(uint32_t rgba) : rgba(rgba) {}

		constexpr explicit Color(const LinearColor& linear);

		static inline constexpr float_t byte_to_float(byte value) { return static_cast<float_t>(value) / 255.f; }
		static inline constexpr byte float_to_byte(float_t value)
		{
			value = value < 0.0f ? 0.0f : (value > 1.0f ? 1.0f : value);
			return static_cast<byte>(value * 255.0f + 0.5f);
		}

		static constexpr Color white() { return {255, 255, 255, 255}; }
		static constexpr Color black() { return {0, 0, 0, 255}; }
		static constexpr Color red() { return {255, 0, 0, 255}; }
		static constexpr Color green() { return {0, 255, 0, 255}; }
		static constexpr Color blue() { return {0, 0, 255, 255}; }
		static constexpr Color yellow() { return {255, 255, 0, 255}; }
		static constexpr Color cyan() { return {0, 255, 255, 255}; }
		static constexpr Color magenta() { return {255, 0, 255, 255}; }
		static constexpr Color transparent() { return {0, 0, 0, 0}; }

		explicit operator Vector4b() const { return Vector4b(r, g, b, a); }
		Color(const Vector4b& v) : r(v[0]), g(v[1]), b(v[2]), a(v[3]) {}

		constexpr Color& operator=(uint32_t value)
		{
			rgba = value;
			return *this;
		}

		constexpr Color& operator=(const Vector4b& v)
		{
			r = v.r;
			g = v.g;
			b = v.b;
			a = v.a;
			return *this;
		}

		constexpr byte& operator[](uint_t i)
		{
			switch (i)
			{
				case 0: return r;
				case 1: return g;
				case 2: return b;
				case 3: return a;
				default: trinex_unreachable_msg("Index out of range");
			}
		}

		constexpr byte operator[](uint_t i) const
		{
			switch (i)
			{
				case 0: return r;
				case 1: return g;
				case 2: return b;
				case 3: return a;
				default: trinex_unreachable_msg("Index out of range");
			}
		}

		constexpr bool operator==(const Color& other) const
		{
			return r == other.r && g == other.g && b == other.b && a == other.a;
		}

		constexpr bool operator!=(const Color& other) const { return !(*this == other); }
	};

	struct LinearColor {
		float_t r, g, b, a;

		constexpr LinearColor() : r(0), g(0), b(0), a(1) {}
		constexpr LinearColor(float_t rr, float_t gg, float_t bb, float_t aa = 1.0f) : r(rr), g(gg), b(bb), a(aa) {}

		constexpr explicit LinearColor(const Color& color)
		    : r(static_cast<float_t>(color.r) / 255.0f), g(static_cast<float_t>(color.g) / 255.0f),
		      b(static_cast<float_t>(color.b) / 255.0f), a(static_cast<float_t>(color.a) / 255.0f)
		{}

		static constexpr LinearColor white() { return {1, 1, 1, 1}; }
		static constexpr LinearColor black() { return {0, 0, 0, 1}; }
		static constexpr LinearColor red() { return {1, 0, 0, 1}; }
		static constexpr LinearColor green() { return {0, 1, 0, 1}; }
		static constexpr LinearColor blue() { return {0, 0, 1, 1}; }

		explicit operator Vector4f() const { return Vector4f(r, g, b, a); }

		LinearColor(const Vector4f& v) : r(v[0]), g(v[1]), b(v[2]), a(v[3]) {}

		LinearColor& operator=(const Vector4f& v)
		{
			r = v.r;
			g = v.g;
			b = v.b;
			a = v.a;
			return *this;
		}

		constexpr float& operator[](uint_t i)
		{
			switch (i)
			{
				case 0: return r;
				case 1: return g;
				case 2: return b;
				case 3: return a;
				default: trinex_unreachable_msg("Index out of range");
			}
		}

		constexpr float operator[](uint_t i) const
		{
			switch (i)
			{
				case 0: return r;
				case 1: return g;
				case 2: return b;
				case 3: return a;
				default: trinex_unreachable_msg("Index out of range");
			}
		}

		constexpr bool operator==(const LinearColor& other) const
		{
			return r == other.r && g == other.g && b == other.b && a == other.a;
		}

		constexpr bool operator!=(const LinearColor& other) const { return !(*this == other); }

		constexpr LinearColor& operator+=(float_t value)
		{
			r += value;
			g += value;
			b += value;
			a += value;
			return *this;
		}

		constexpr LinearColor& operator+=(const Vector4f& value)
		{
			r += value.r;
			g += value.g;
			b += value.b;
			a += value.a;
			return *this;
		}

		constexpr LinearColor& operator+=(const LinearColor& color)
		{
			r += color.r;
			g += color.g;
			b += color.b;
			a += color.a;
			return *this;
		}

		constexpr LinearColor& operator-=(float_t value)
		{
			r -= value;
			g -= value;
			b -= value;
			a -= value;
			return *this;
		}

		constexpr LinearColor& operator-=(const Vector4f& value)
		{
			r -= value.r;
			g -= value.g;
			b -= value.b;
			a -= value.a;
			return *this;
		}

		constexpr LinearColor& operator-=(const LinearColor& color)
		{
			r -= color.r;
			g -= color.g;
			b -= color.b;
			a -= color.a;
			return *this;
		}

		constexpr LinearColor& operator*=(float_t value)
		{
			r *= value;
			g *= value;
			b *= value;
			a *= value;
			return *this;
		}

		constexpr LinearColor& operator*=(const Vector4f& value)
		{
			r *= value.r;
			g *= value.g;
			b *= value.b;
			a *= value.a;
			return *this;
		}

		constexpr LinearColor& operator*=(const LinearColor& color)
		{
			r *= color.r;
			g *= color.g;
			b *= color.b;
			a *= color.a;
			return *this;
		}

		constexpr LinearColor& operator/=(float_t value)
		{
			r *= value;
			g *= value;
			b *= value;
			a *= value;
			return *this;
		}

		constexpr LinearColor& operator/=(const Vector4f& value)
		{
			r *= value.r;
			g *= value.g;
			b *= value.b;
			a *= value.a;
			return *this;
		}

		constexpr LinearColor& operator/=(const LinearColor& color)
		{
			r *= color.r;
			g *= color.g;
			b *= color.b;
			a *= color.a;
			return *this;
		}
	};

	constexpr Color::Color(const LinearColor& linear)
	    : r(Color::float_to_byte(linear.r)), g(Color::float_to_byte(linear.g)), b(Color::float_to_byte(linear.b)),
	      a(Color::float_to_byte(linear.a))
	{}
}// namespace Engine
