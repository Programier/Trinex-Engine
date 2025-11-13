#pragma once

namespace Engine
{
	struct Angle {
		float value;

		constexpr Angle() : value(0.0f) {}
		constexpr Angle(float v) : value(v) {}

		constexpr operator float() const { return value; }

		constexpr Angle operator+(Angle rhs) const { return Angle(value + rhs.value); }
		constexpr Angle operator-(Angle rhs) const { return Angle(value - rhs.value); }
		constexpr Angle operator*(float rhs) const { return Angle(value * rhs); }
		constexpr Angle operator/(float rhs) const { return Angle(value / rhs); }

		constexpr Angle& operator+=(Angle rhs)
		{
			value += rhs.value;
			return *this;
		}

		constexpr Angle& operator-=(Angle rhs)
		{
			value -= rhs.value;
			return *this;
		}

		constexpr Angle& operator*=(float rhs)
		{
			value *= rhs;
			return *this;
		}

		constexpr Angle& operator/=(float rhs)
		{
			value /= rhs;
			return *this;
		}

		constexpr Angle operator-() const { return Angle(-value); }

		constexpr bool operator==(Angle rhs) const { return value == rhs.value; }
		constexpr bool operator!=(Angle rhs) const { return value != rhs.value; }
		constexpr bool operator<(Angle rhs) const { return value < rhs.value; }
		constexpr bool operator>(Angle rhs) const { return value > rhs.value; }
		constexpr bool operator<=(Angle rhs) const { return value <= rhs.value; }
		constexpr bool operator>=(Angle rhs) const { return value >= rhs.value; }
	};
}// namespace Engine
