#include <Engine/Render/batched_primitives.hpp>
#include <Engine/aabb.hpp>
#include <Engine/ray.hpp>

namespace Engine
{
	AABB_3Df::AABB_3Df(const Vector3f& min, const Vector3f& max)
	{
		minmax(min, max);
	}

	AABB_3Df::AABB_3Df(const AABB_3Df& other)            = default;
	AABB_3Df& AABB_3Df::operator=(const AABB_3Df& other) = default;

	const Vector3f& AABB_3Df::min() const
	{
		return m_min;
	}

	const Vector3f& AABB_3Df::max() const
	{
		return m_max;
	}

	Vector3f AABB_3Df::extents() const
	{
		return m_max - center();
	}

	Vector3f AABB_3Df::center() const
	{
		return m_min + (m_max - m_min) / 2.0f;
	}

	AABB_3Df& AABB_3Df::center(const Vector3f& position)
	{
		Vector3f half_size = m_max - center();
		m_max              = position + half_size;
		m_min              = position - half_size;
		return *this;
	}

	AABB_3Df& AABB_3Df::min(const Vector3f& new_min)
	{
		return minmax(new_min, m_max);
	}

	AABB_3Df& AABB_3Df::max(const Vector3f& new_max)
	{
		return minmax(m_min, new_max);
	}

	AABB_3Df& AABB_3Df::minmax(const Vector3f& new_min, const Vector3f& new_max)
	{
		m_min = glm::min(new_min, new_max);
		m_max = glm::max(new_min, new_max);

		return *this;
	}

	Vector3f AABB_3Df::size() const
	{
		return m_max - m_min;
	}

	AABB_3Df AABB_3Df::apply_transform(const Matrix4f& matrix) const
	{
		auto box_center  = center();
		auto box_extents = extents();

		Vector3f t_center = Vector3f(matrix * Vector4f(box_center, 1.0));
		Matrix3f abs_mat  = Matrix3f(glm::abs(Vector3f(matrix[0])), glm::abs(Vector3f(matrix[1])), glm::abs(Vector3f(matrix[2])));
		Vector3f t_extents = abs_mat * box_extents;
		return AABB_3Df(t_center - t_extents, t_center + t_extents);
	}

	const AABB_3Df& AABB_3Df::write_to_batcher(BatchedLines& batcher, const Color& color) const
	{
		using Vertex = BatchedLines::Vertex;

		Vector3f min = m_min - Vector3f(0.001f, 0.001f, 0.001f);
		Vector3f max = m_max + Vector3f(0.001f, 0.001f, 0.001f);

		batcher.add_line(Vertex({min.x, min.y, min.z}, color, 3.0), Vertex({max.x, min.y, min.z}, color, 3.0));
		batcher.add_line(Vertex({min.x, max.y, min.z}, color, 3.0), Vertex({max.x, max.y, min.z}, color, 3.0));
		batcher.add_line(Vertex({min.x, min.y, max.z}, color, 3.0), Vertex({max.x, min.y, max.z}, color, 3.0));
		batcher.add_line(Vertex({min.x, max.y, max.z}, color, 3.0), Vertex({max.x, max.y, max.z}, color, 3.0));

		batcher.add_line(Vertex({min.x, min.y, min.z}, color, 3.0), Vertex({min.x, max.y, min.z}, color, 3.0));
		batcher.add_line(Vertex({max.x, min.y, min.z}, color, 3.0), Vertex({max.x, max.y, min.z}, color, 3.0));
		batcher.add_line(Vertex({min.x, min.y, max.z}, color, 3.0), Vertex({min.x, max.y, max.z}, color, 3.0));
		batcher.add_line(Vertex({max.x, min.y, max.z}, color, 3.0), Vertex({max.x, max.y, max.z}, color, 3.0));

		batcher.add_line(Vertex({min.x, min.y, min.z}, color, 3.0), Vertex({min.x, min.y, max.z}, color, 3.0));
		batcher.add_line(Vertex({max.x, min.y, min.z}, color, 3.0), Vertex({max.x, min.y, max.z}, color, 3.0));
		batcher.add_line(Vertex({min.x, max.y, min.z}, color, 3.0), Vertex({min.x, max.y, max.z}, color, 3.0));
		batcher.add_line(Vertex({max.x, max.y, min.z}, color, 3.0), Vertex({max.x, max.y, max.z}, color, 3.0));
		return *this;
	}

	bool AABB_3Df::inside(const AABB_3Df& other) const
	{
		return (m_min.x >= other.m_min.x && m_max.x <= other.m_max.x) && (m_min.y >= other.m_min.y && m_max.y <= other.m_max.y) &&
		       (m_min.z >= other.m_min.z && m_max.z <= other.m_max.z);
	}

	bool AABB_3Df::intersect(const AABB_3Df& other) const
	{
		return (m_min.x <= other.m_max.x && m_max.x >= other.m_min.x) && (m_min.y <= other.m_max.y && m_max.y >= other.m_min.y) &&
		       (m_min.z <= other.m_max.z && m_max.z >= other.m_min.z);
	}

	Vector2f AABB_3Df::intersect(const Ray& ray) const
	{
		const Vector3f& origin    = ray.origin();
		const Vector3f& direction = ray.direction();

		Vector3f t_min = (m_min - origin) / direction;
		Vector3f t_max = (m_max - origin) / direction;
		Vector3f t1    = glm::min(t_min, t_max);
		Vector3f t2    = glm::max(t_min, t_max);
		float t_near   = glm::max(glm::max(t1.x, t1.y), t1.z);
		float t_far    = glm::min(glm::min(t2.x, t2.y), t2.z);
		return Vector2f(t_near, t_far);
	}

	Vector2f Ray::intersect(const AABB_3Df& aabb) const
	{
		return aabb.intersect(*this);
	}

	bool AABB_3Df::outside(const AABB_3Df& other) const
	{
		return (m_min.x > other.m_max.x || m_max.x < other.m_min.x) || (m_min.y > other.m_max.y || m_max.y < other.m_min.y) ||
		       (m_min.z > other.m_max.z || m_max.z < other.m_min.z);
	}

	bool AABB_3Df::contains(const Vector3f& point) const
	{
		return point.x > m_min.x && point.x < m_max.x &&//
		       point.y > m_min.y && point.y < m_max.y &&//
		       point.z > m_min.z && point.z < m_max.z;
	}

	template<typename T>
	static AABB_3Df mult_op(const AABB_3Df& self, const T& value)
	{
		Vector3f box_center = self.center();
		Vector3f offset     = (self.max() - box_center) * value;
		return AABB_3Df(box_center - offset, box_center + offset);
	}

	template<typename T>
	static AABB_3Df div_op(const AABB_3Df& self, const T& value)
	{
		Vector3f box_center = self.center();
		Vector3f offset     = (self.max() - box_center) / value;
		return AABB_3Df(box_center - offset, box_center + offset);
	}

	AABB_3Df AABB_3Df::operator*(float value) const
	{
		return mult_op(*this, value);
	}

	AABB_3Df AABB_3Df::operator/(float value) const
	{
		return div_op(*this, value);
	}

	AABB_3Df AABB_3Df::operator*(const Vector3f& scale) const
	{
		return mult_op(*this, scale);
	}

	AABB_3Df AABB_3Df::operator/(const Vector3f& scale) const
	{
		return div_op(*this, scale);
	}

	AABB_3Df& AABB_3Df::operator+=(const Vector3f& offset)
	{
		m_max += offset;
		m_min += offset;
		return *this;
	}

	AABB_3Df& AABB_3Df::operator-=(const Vector3f& offset)
	{
		m_max -= offset;
		m_min -= offset;
		return *this;
	}

	AABB_3Df AABB_3Df::operator+(const Vector3f& offset) const
	{
		AABB_3Df new_box = (*this);
		return new_box += offset;
	}

	AABB_3Df AABB_3Df::operator-(const Vector3f& offset) const
	{
		AABB_3Df new_box = (*this);
		return new_box -= offset;
	}

	ENGINE_EXPORT AABB_3Df operator*(float value, const AABB_3Df& box)
	{
		return box * value;
	}

	ENGINE_EXPORT AABB_3Df operator*(const Vector3f& scale, const AABB_3Df& box)
	{
		return box * scale;
	}

	ENGINE_EXPORT AABB_3Df operator+(const Vector3f& offset, AABB_3Df self)
	{
		return self += offset;
	}

	ENGINE_EXPORT AABB_3Df operator-(const Vector3f& offset, AABB_3Df self)
	{
		Vector3f tmp = self.m_max;
		self.m_max   = offset - self.m_min;
		self.m_min   = offset - tmp;
		return self;
	}

}// namespace Engine
