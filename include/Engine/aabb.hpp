#pragma once
#include <Core/engine_types.hpp>
#include <Core/types/color.hpp>

namespace Engine
{
	class Ray;

	class ENGINE_EXPORT AABB_3Df
	{
	private:
		Vector3f m_min;
		Vector3f m_max;


	public:
		AABB_3Df(const Vector3f& min = {}, const Vector3f& max = {});
		AABB_3Df(const AABB_3Df& other);
		AABB_3Df& operator=(const AABB_3Df& other);

		const Vector3f& min() const;
		const Vector3f& max() const;
		Vector3f extents() const;
		Vector3f center() const;
		AABB_3Df& center(const Vector3f& position);

		AABB_3Df& min(const Vector3f& new_min);
		AABB_3Df& max(const Vector3f& new_max);
		AABB_3Df& minmax(const Vector3f& new_min, const Vector3f& new_max);
		Vector3f size() const;
		AABB_3Df apply_transform(const Matrix4f& matrix) const;
		const AABB_3Df& write_to_batcher(class BatchedLines& batcher, const Color& color = {255, 255, 255, 255}) const;

		bool inside(const AABB_3Df& other) const;
		bool intersect(const AABB_3Df& other) const;
		Vector2f intersect(const Ray& ray) const;
		bool outside(const AABB_3Df& other) const;
		bool contains(const Vector3f& point) const;

		AABB_3Df operator*(float value) const;
		AABB_3Df operator/(float value) const;
		AABB_3Df operator*(const Vector3f& scale) const;
		AABB_3Df operator/(const Vector3f& scale) const;

		AABB_3Df& operator+=(const Vector3f& offset);
		AABB_3Df& operator-=(const Vector3f& offset);
		AABB_3Df operator+(const Vector3f& offset) const;
		AABB_3Df operator-(const Vector3f& offset) const;

		friend AABB_3Df operator*(float value, const AABB_3Df&);
		friend AABB_3Df operator*(const Vector3f&, const AABB_3Df&);
		friend AABB_3Df operator+(const Vector3f&, AABB_3Df);
		friend AABB_3Df operator-(const Vector3f&, AABB_3Df);
	};

	ENGINE_EXPORT AABB_3Df operator*(float value, const AABB_3Df&);
	ENGINE_EXPORT AABB_3Df operator*(const Vector3f&, const AABB_3Df&);
	ENGINE_EXPORT AABB_3Df operator+(const Vector3f&, AABB_3Df);
	ENGINE_EXPORT AABB_3Df operator-(const Vector3f&, AABB_3Df);
}// namespace Engine
