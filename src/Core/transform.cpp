#include <Core/archive.hpp>
#include <Core/buffer_manager.hpp>
#include <Core/class.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/property.hpp>
#include <Core/string_functions.hpp>
#include <Core/struct.hpp>
#include <Core/transform.hpp>
#include <Engine/ActorComponents/scene_component.hpp>
#include <ScriptEngine/registrar.hpp>
#include <glm/gtx/matrix_decompose.hpp>

namespace Engine
{
	const Transform Transform::transform_zero;

	implement_struct(Engine, Transform)
	{
		Struct* self                = static_struct_instance();
		static auto on_prop_changed = [](void* object) {
			Transform* transform  = reinterpret_cast<Transform*>(object);
			transform->m_is_dirty = true;
		};

		auto location_prop = new Vec3Property("Location", "Location component of transform", &Transform::m_location);
		auto rotation_prop = new Vec3Property("Rotation", "Rotation component of transform", &Transform::m_rotation);
		auto scale_prop    = new Vec3Property("Scale", "Scale component of transform", &Transform::m_scale);

		location_prop->on_prop_changed.push(on_prop_changed);
		rotation_prop->on_prop_changed.push(on_prop_changed);
		scale_prop->on_prop_changed.push(on_prop_changed);
		self->add_properties(location_prop, rotation_prop, scale_prop);
	}

	Transform::Transform(const Vector3D& location, const Vector3D& rotation, const Vector3D& scale)
	    : m_location(location), m_rotation(rotation), m_scale(scale), m_is_dirty(true)
	{}

	Transform::Transform(const Vector3D& location, const Quaternion& rotation, const Vector3D& scale)
	    : Transform(location, glm::degrees(glm::eulerAngles(rotation)), scale)
	{}


	Transform::Transform(const Matrix4f& matrix)
	{
		(*this) = matrix;
	}

	default_copy_constructors_cpp(Transform);

	Transform& Transform::operator=(const Matrix4f& matrix)
	{
		static Quaternion quat;
		static Vector3D skew;
		static Vector4D perspective;
		glm::decompose(matrix, m_scale, quat, m_location, skew, perspective);
		rotation(quat);
		m_matrix   = matrix;
		m_is_dirty = false;
		return *this;
	}

	Matrix4f Transform::translation_matrix() const
	{
		return glm::translate(Matrix4f(1.f), m_location);
	}

	Matrix4f Transform::rotation_matrix() const
	{
		return glm::mat4_cast(Quaternion(glm::radians(m_rotation)));
	}

	Matrix4f Transform::scale_matrix() const
	{
		return glm::scale(Matrix4f(1.f), m_scale);
	}

	const Vector3D& Transform::location() const
	{
		return m_location;
	}

	const Vector3D& Transform::rotation() const
	{
		return m_rotation;
	}

	Quaternion Transform::quaternion() const
	{
		return angles_to_quaternion(m_rotation);
	}

	Quaternion Transform::angles_to_quaternion(const Vector3D& angles)
	{
		return Quaternion(glm::radians(angles));
	}

	Vector3D Transform::quaternion_to_angles(const Quaternion& quat)
	{
		return glm::degrees(glm::eulerAngles(quat));
	}

	Transform& Transform::location(const Vector3D& new_location)
	{
		m_location = new_location;
		m_is_dirty = true;
		return *this;
	}

	Transform& Transform::rotation(const Quaternion& new_rotation)
	{
		return rotation(glm::degrees(glm::eulerAngles(new_rotation)));
	}

	Transform& Transform::rotation(const Vector3D& new_rotation)
	{
		m_rotation = new_rotation;
		m_is_dirty = true;
		return *this;
	}

	Transform& Transform::scale(const Vector3D& new_scale)
	{
		m_scale    = new_scale;
		m_is_dirty = true;
		return *this;
	}

	const Vector3D& Transform::scale() const
	{
		return m_scale;
	}

	Transform& Transform::add_location(const Vector3D& delta)
	{
		m_location += delta;
		m_is_dirty = true;
		return *this;
	}

	Transform& Transform::add_rotation(const Vector3D& delta)
	{
		m_rotation += delta;
		m_is_dirty = true;
		return *this;
	}

	Transform& Transform::add_rotation(const Quaternion& delta)
	{
		return add_rotation(glm::degrees(glm::eulerAngles(delta)));
	}

	Transform& Transform::add_scale(const Vector3D& delta)
	{
		m_scale *= delta;
		m_is_dirty = true;
		return *this;
	}

	Transform& Transform::look_at(const Vector3D& position, const Vector3D& up)
	{
		Transform::operator=(glm::inverse(glm::lookAt(location(), position, up)) * scale_matrix());
		return *this;
	}

	Transform Transform::operator+(const Transform& other) const
	{
		Transform new_transform = *this;
		new_transform.add_location(other.location());
		new_transform.add_rotation(other.rotation());
		new_transform.add_scale(other.scale());
		return new_transform;
	}

	Transform Transform::operator-(const Transform& other) const
	{
		Transform new_transform = *this;
		new_transform.add_location(-other.location());
		new_transform.add_rotation(-other.m_rotation);
		new_transform.add_scale(1.f / other.scale());
		return new_transform;
	}


	Transform& Transform::operator+=(const Transform& other)
	{
		add_location(other.location());
		add_rotation(other.rotation());
		add_scale(other.scale());
		return *this;
	}

	Transform& Transform::operator-=(const Transform& other)
	{
		add_location(-other.location());
		add_rotation(-other.rotation());
		add_scale(1.f / other.scale());
		return *this;
	}


	Transform& Transform::operator*=(const Transform& other)
	{
		return *(new (this) Transform(matrix() * other.matrix()));
	}

	Transform& Transform::operator/=(const Transform& other)
	{
		return *(new (this) Transform(matrix() * glm::inverse(other.matrix())));
	}

	Transform Transform::operator*(const Transform& other) const
	{
		return Transform(matrix() * other.matrix());
	}

	Transform Transform::operator/(const Transform& other) const
	{
		return Transform(matrix() * glm::inverse(other.matrix()));
	}


	const Matrix4f& Transform::matrix() const
	{
		if (m_is_dirty)
		{
			m_matrix = glm::translate(Matrix4f(1.f), m_location);
			m_matrix *= glm::mat4_cast(Quaternion(glm::radians((m_rotation))));
			m_matrix   = glm::scale(m_matrix, m_scale);
			m_is_dirty = false;
		}
		return m_matrix;
	}

	Vector3D Transform::vector_of(const Vector3D& dir) const
	{
		return glm::normalize(glm::mat3_cast(Quaternion(glm::radians(m_rotation))) * dir);
	}

	Vector3D Transform::forward_vector() const
	{
		return vector_of(Constants::forward_vector);
	}

	Vector3D Transform::right_vector() const
	{
		return vector_of(Constants::right_vector);
	}

	Vector3D Transform::up_vector() const
	{
		return vector_of(Constants::up_vector);
	}

	String Transform::as_string() const
	{
		return Strings::format("Location: {}, {}, {}\n"
		                       "Rotation: {}, {}, {}\n"
		                       "Scale: {}, {}, {}",                     //
		                       m_location.x, m_location.y, m_location.z,//
		                       m_rotation.x, m_rotation.y, m_rotation.z,//
		                       m_scale.x, m_scale.y, m_scale.z);
	}

	bool Transform::is_dirty() const
	{
		return m_is_dirty;
	}

	const Transform& Transform::mark_dirty() const
	{
		m_is_dirty = true;
		return *this;
	}

	bool operator&(Archive& ar, Transform& t)
	{
		ar & t.m_location;
		ar & t.m_scale;
		ar & t.m_rotation;
		return static_cast<bool>(ar);
	}


	static Transform& op_assign(Transform* _this, const Transform& obj)
	{
		(*_this) = obj;
		return *_this;
	}

	static void on_init()
	{
		ScriptClassRegistrar registrar = ScriptClassRegistrar::value_class("Engine::Transform", sizeof(Transform));

		registrar.behave(ScriptClassBehave::Construct, "void f()", ScriptClassRegistrar::constructor<Transform>,
		                 ScriptCallConv::CDeclObjFirst);
		registrar.behave(ScriptClassBehave::Construct, "void f(const Engine::Transform&)",
		                 ScriptClassRegistrar::constructor<Transform, const Transform&>, ScriptCallConv::CDeclObjFirst);
		registrar.behave(ScriptClassBehave::Destruct, "void f()", ScriptClassRegistrar::destructor<Transform>,
		                 ScriptCallConv::CDeclObjFirst);

		registrar.opfunc("Engine::Transform& opAssign(const Engine::Transform&)", op_assign, ScriptCallConv::CDeclObjFirst);
	}

	static ReflectionInitializeController init(on_init, "Engine::Transform",
	                                           {"Engine::Matrix", "Engine::Vector", "Engine::Quaternion"});
}// namespace Engine
