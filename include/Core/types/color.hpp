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
				u8 r, g, b, a;
			};

			u32 rgba;
		};

		constexpr Color() : r(0), g(0), b(0), a(255) {}
		constexpr Color(u8 r, u8 g, u8 b, u8 a = 255) : r(r), g(g), b(b), a(a) {}
		constexpr Color(u32 rgba) : rgba(rgba) {}

		constexpr explicit Color(const LinearColor& linear);

		static inline constexpr f32 byte_to_float(u8 value) { return static_cast<f32>(value) / 255.f; }
		static inline constexpr u8 float_to_byte(f32 value)
		{
			value = value < 0.0f ? 0.0f : (value > 1.0f ? 1.0f : value);
			return static_cast<u8>(value * 255.0f + 0.5f);
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

		constexpr Color& operator=(u32 value)
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

		constexpr u8& operator[](u32 i)
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

		constexpr u8 operator[](u32 i) const
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
		f32 r, g, b, a;

		constexpr LinearColor() : r(0), g(0), b(0), a(1) {}
		constexpr LinearColor(f32 rr, f32 gg, f32 bb, f32 aa = 1.0f) : r(rr), g(gg), b(bb), a(aa) {}

		constexpr explicit LinearColor(const Color& color)
		    : r(static_cast<f32>(color.r) / 255.0f), g(static_cast<f32>(color.g) / 255.0f), b(static_cast<f32>(color.b) / 255.0f),
		      a(static_cast<f32>(color.a) / 255.0f)
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

		constexpr float& operator[](u32 i)
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

		constexpr float operator[](u32 i) const
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

		constexpr LinearColor& operator+=(f32 value)
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

		constexpr LinearColor& operator-=(f32 value)
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

		constexpr LinearColor& operator*=(f32 value)
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

		constexpr LinearColor& operator/=(f32 value)
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
