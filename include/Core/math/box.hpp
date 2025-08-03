#pragma once
#include <Core/math/matrix.hpp>
#include <Core/math/vector.hpp>

namespace Engine
{
	template<size_t N, typename T>
	class Box
	{
	public:
		static_assert(std::is_signed_v<T>, "T must be signed type!");

		using VectorType = VectorNT<N, T>;
		using MatrixType = MatrixNT<N + 1, T>;

		VectorType min;
		VectorType max;

	public:
		constexpr Box(const VectorType& min = {}, const VectorType& max = {}) : min(min), max(max) {}
		constexpr Box(const Box& other)            = default;
		constexpr Box& operator=(const Box& other) = default;

		constexpr VectorType extents() const { return (max - min) / static_cast<T>(2); }
		constexpr VectorType center() const { return (min + max) / static_cast<T>(2); }
		constexpr Box& center(const Vector3f& position)
		{
			VectorType extends = extents();
			max                = position + extends;
			min                = position - extends;
			return *this;
		}

		constexpr VectorType size() const { return max - min; }

		constexpr Box transform(const MatrixType& matrix) const
		{
			VectorType box_center  = center();
			VectorType box_extents = extents();

			VectorType center      = VectorType(matrix * VectorNT<N + 1, T>(box_center, static_cast<T>(1)));
			MatrixNT<N, T> abs_mat = MatrixNT<N, T>(glm::abs(VectorType(matrix[0])), glm::abs(VectorType(matrix[1])),
			                                        glm::abs(VectorType(matrix[2])));
			VectorType extents     = abs_mat * box_extents;
			return Box(center - extents, center + extents);
		}

		constexpr bool inside(const Box& other) const
		{
			uint_t len = VectorType::length();

			const T* min_ptr1 = &min.x;
			const T* max_ptr1 = &max.x;

			const T* min_ptr2 = &other.min.x;
			const T* max_ptr2 = &other.max.x;

			for (uint_t i = 0; i < len; ++i)
			{
				float min1 = min_ptr1[i];
				float min2 = min_ptr2[i];

				float max1 = max_ptr1[i];
				float max2 = max_ptr2[i];

				if (min1 < min2 || max1 > max2)
					return false;
			}

			return true;
		}

		constexpr bool intersect(const Box& other) const
		{
			uint_t len = VectorType::length();

			const T* min_ptr1 = &min.x;
			const T* max_ptr1 = &max.x;

			const T* min_ptr2 = &other.min.x;
			const T* max_ptr2 = &other.max.x;

			for (uint_t i = 0; i < len; ++i)
			{
				float min1 = min_ptr1[i];
				float min2 = min_ptr2[i];

				float max1 = max_ptr1[i];
				float max2 = max_ptr2[i];

				if (min1 > max2 || min2 > max1)
					return false;
			}

			return true;
		}

		constexpr bool outside(const Box& other) const { return !intersect(other); }
		constexpr bool contains(const VectorType& point) const
		{
			uint_t len = VectorType::length();

			const T* min_ptr   = &min.x;
			const T* max_ptr   = &max.x;
			const T* value_ptr = &point.x;

			for (uint i = 0; i < len; ++i)
			{
				const T value = value_ptr[i];

				if (value < min_ptr[i] || value > max_ptr[i])
					return false;
			}

			return true;
		}

		constexpr Box operator+(T value) const { return Box(min + value, max + value); }
		constexpr Box operator-(T value) const { return Box(min - value, max - value); }
		constexpr Box operator*(T value) const { return Box(min * value, max * value); }
		constexpr Box operator/(T value) const { return Box(min / value, max / value); }
		constexpr Box operator+(const VectorType& value) const { return Box{min + value, max + value}; }
		constexpr Box operator-(const VectorType& value) const { return Box{min - value, max - value}; }
		constexpr Box operator*(const VectorType& value) const { return Box(min * value, max * value); }
		constexpr Box operator/(const VectorType& value) const { return Box(min / value, max / value); }


		constexpr Box& operator+=(T value)
		{
			min += value;
			max += value;
			return *this;
		}

		constexpr Box& operator-=(T value)
		{
			min -= value;
			max -= value;
			return *this;
		}

		constexpr Box& operator*=(T value)
		{
			min *= value;
			max *= value;
			return *this;
		}

		constexpr Box& operator/=(T value)
		{
			min /= value;
			max /= value;
			return *this;
		}

		constexpr Box& operator+=(const VectorType& value)
		{
			min += value;
			max += value;
			return *this;
		}

		constexpr Box& operator-=(const VectorType& value)
		{
			min -= value;
			max -= value;
			return *this;
		}

		constexpr Box& operator*=(const VectorType& value)
		{
			min *= value;
			max *= value;
			return *this;
		}

		constexpr Box& operator/=(const VectorType& value)
		{
			min /= value;
			max /= value;
			return *this;
		}
	};
}// namespace Engine
