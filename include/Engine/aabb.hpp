#pragma once
#include <Core/engine_types.hpp>

namespace Engine
{
	class Ray;

	class ENGINE_EXPORT AABB_3Df
	{
	private:
		Vector3D m_min;
		Vector3D m_max;


	public:
		AABB_3Df(const Vector3D& min = {}, const Vector3D& max = {});
		AABB_3Df(const AABB_3Df& other);
		AABB_3Df& operator=(const AABB_3Df& other);

		const Vector3D& min() const;
		const Vector3D& max() const;
		Vector3D extents() const;
		Vector3D center() const;
		AABB_3Df& center(const Vector3D& position);

		AABB_3Df& min(const Vector3D& new_min);
		AABB_3Df& max(const Vector3D& new_max);
		AABB_3Df& minmax(const Vector3D& new_min, const Vector3D& new_max);
		Vector3D size() const;
		AABB_3Df apply_transform(const Matrix4f& matrix) const;
		const AABB_3Df& write_to_batcher(class BatchedLines& batcher,
										 const ByteColor& color = {255, 255, 255, 255}) const;

		bool inside(const AABB_3Df& other) const;
		bool intersect(const AABB_3Df& other) const;
		Vector2D intersect(const Ray& ray) const;
		bool outside(const AABB_3Df& other) const;
		bool contains(const Vector3D& point) const;

		AABB_3Df operator*(float value) const;
		AABB_3Df operator/(float value) const;
		AABB_3Df operator*(const Vector3D& scale) const;
		AABB_3Df operator/(const Vector3D& scale) const;

		AABB_3Df& operator+=(const Vector3D& offset);
		AABB_3Df& operator-=(const Vector3D& offset);
		AABB_3Df operator+(const Vector3D& offset) const;
		AABB_3Df operator-(const Vector3D& offset) const;

		friend AABB_3Df operator*(float value, const AABB_3Df&);
		friend AABB_3Df operator*(const Vector3D&, const AABB_3Df&);
		friend AABB_3Df operator+(const Vector3D&, AABB_3Df);
		friend AABB_3Df operator-(const Vector3D&, AABB_3Df);
	};

	ENGINE_EXPORT AABB_3Df operator*(float value, const AABB_3Df&);
	ENGINE_EXPORT AABB_3Df operator*(const Vector3D&, const AABB_3Df&);
	ENGINE_EXPORT AABB_3Df operator+(const Vector3D&, AABB_3Df);
	ENGINE_EXPORT AABB_3Df operator-(const Vector3D&, AABB_3Df);
}// namespace Engine
