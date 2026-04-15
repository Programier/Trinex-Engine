#pragma once
#include <Core/math/matrix.hpp>
#include <Core/math/vector.hpp>
#include <glm/common.hpp>

namespace Trinex
{
	template<size_t N, typename T>
	class BoxNT
	{
	public:
		static_assert(std::is_signed_v<T>, "T must be signed type!");

		using VectorType = VectorNT<N, T>;
		using MatrixType = MatrixNT<N + 1, T>;

		VectorType min;
		VectorType max;

	public:
		constexpr BoxNT(const VectorType& min = {}, const VectorType& max = {}) : min(min), max(max) {}

		template<typename ArgumentType>
		    requires(std::is_floating_point_v<ArgumentType> && !std::is_floating_point_v<T>)
		constexpr BoxNT(const BoxNT<N, ArgumentType>& other) : min(glm::floor(other.min)), max(glm::ceil(other.max))
		{}

		template<typename ArgumentType>
		constexpr BoxNT(const BoxNT<N, ArgumentType>& other) : min(other.min), max(other.max)
		{}

		template<typename ArgumentType>
		    requires(std::is_floating_point_v<ArgumentType> && !std::is_floating_point_v<T>)
		constexpr BoxNT& operator=(const BoxNT& other)
		{
			min = glm::floor(other.min);
			max = glm::ceil(other.max);
		}

		template<typename ArgumentType>
		constexpr BoxNT& operator=(const BoxNT& other)
		{
			min = other.min;
			max = other.max;
		}

		constexpr VectorType extents() const { return (max - min) / static_cast<T>(2); }
		constexpr VectorType center() const { return (min + max) / static_cast<T>(2); }
		constexpr BoxNT& center(const Vector3f& position)
		{
			VectorType extends = extents();
			max                = position + extends;
			min                = position - extends;
			return *this;
		}

		constexpr VectorType size() const { return max - min; }
		constexpr T radius() const { return glm::length(extents()); }

		constexpr BoxNT transform(const MatrixType& matrix) const
		{
			VectorType BoxNT_center  = center();
			VectorType BoxNT_extents = extents();

			VectorType center      = VectorType(matrix * VectorNT<N + 1, T>(BoxNT_center, static_cast<T>(1)));
			MatrixNT<N, T> abs_mat = MatrixNT<N, T>(glm::abs(VectorType(matrix[0])), glm::abs(VectorType(matrix[1])),
			                                        glm::abs(VectorType(matrix[2])));
			VectorType extents     = abs_mat * BoxNT_extents;
			return BoxNT(center - extents, center + extents);
		}

		constexpr bool inside(const BoxNT& other) const
		{
			constexpr u32 len = VectorType::length();

			const T* min_ptr1 = &min.x;
			const T* max_ptr1 = &max.x;

			const T* min_ptr2 = &other.min.x;
			const T* max_ptr2 = &other.max.x;

			for (u32 i = 0; i < len; ++i)
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

		constexpr bool intersect(const BoxNT& other) const
		{
			constexpr u32 len = VectorType::length();

			const T* min_ptr1 = &min.x;
			const T* max_ptr1 = &max.x;

			const T* min_ptr2 = &other.min.x;
			const T* max_ptr2 = &other.max.x;

			for (u32 i = 0; i < len; ++i)
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

		constexpr bool outside(const BoxNT& other) const { return !intersect(other); }
		constexpr bool contains(const VectorType& point) const
		{
			constexpr u32 len = VectorType::length();

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
	};
}// namespace Trinex
