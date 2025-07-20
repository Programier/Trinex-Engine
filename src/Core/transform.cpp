#include <Core/archive.hpp>
#include <Core/buffer_manager.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/reflection/property.hpp>
#include <Core/reflection/struct.hpp>
#include <Core/string_functions.hpp>
#include <Core/transform.hpp>
#include <Engine/ActorComponents/scene_component.hpp>
#include <ScriptEngine/registrar.hpp>
#include <glm/gtx/matrix_decompose.hpp>

namespace Engine
{
	const Transform Transform::transform_zero;

	trinex_implement_struct(Engine::Transform, 0)
	{
		auto* self = static_reflection();

		auto on_prop_changed = [](const Refl::PropertyChangedEvent& event) { event.context_as<Transform>()->m_is_dirty = true; };

		auto& location = *trinex_refl_prop(self, This, m_location);
		auto& rotation = *trinex_refl_prop(self, This, m_rotation);
		auto& scale    = *trinex_refl_prop(self, This, m_scale);

		location.display_name("Location").tooltip("Location component of transform");
		rotation.display_name("Rotation").tooltip("Rotation component of transform");
		scale.display_name("Scale").tooltip("Scale component of transform");

		location.add_change_listener(on_prop_changed);
		rotation.add_change_listener(on_prop_changed);
		scale.add_change_listener(on_prop_changed);
	}

	Transform::Transform(const Vector3f& location, const Vector3f& rotation, const Vector3f& scale)
	    : m_location(location), m_rotation(rotation), m_scale(scale), m_is_dirty(true)
	{}

	Transform::Transform(const Vector3f& location, const Quaternion& rotation, const Vector3f& scale)
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
		static Vector3f skew;
		static Vector4f perspective;
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

	const Vector3f& Transform::location() const
	{
		return m_location;
	}

	const Vector3f& Transform::rotation() const
	{
		return m_rotation;
	}

	Quaternion Transform::quaternion() const
	{
		return angles_to_quaternion(m_rotation);
	}

	Quaternion Transform::angles_to_quaternion(const Vector3f& angles)
	{
		return Quaternion(glm::radians(angles));
	}

	Vector3f Transform::quaternion_to_angles(const Quaternion& quat)
	{
		return glm::degrees(glm::eulerAngles(quat));
	}

	Transform& Transform::location(const Vector3f& new_location)
	{
		m_location = new_location;
		m_is_dirty = true;
		return *this;
	}

	Transform& Transform::rotation(const Quaternion& new_rotation)
	{
		return rotation(glm::degrees(glm::eulerAngles(new_rotation)));
	}

	Transform& Transform::rotation(const Vector3f& new_rotation)
	{
		m_rotation = new_rotation;
		m_is_dirty = true;
		return *this;
	}

	Transform& Transform::scale(const Vector3f& new_scale)
	{
		m_scale    = new_scale;
		m_is_dirty = true;
		return *this;
	}

	const Vector3f& Transform::scale() const
	{
		return m_scale;
	}

	Transform& Transform::add_location(const Vector3f& delta)
	{
		m_location += delta;
		m_is_dirty = true;
		return *this;
	}

	Transform& Transform::add_rotation(const Vector3f& delta)
	{
		m_rotation += delta;
		m_is_dirty = true;
		return *this;
	}

	Transform& Transform::add_rotation(const Quaternion& delta)
	{
		return add_rotation(glm::degrees(glm::eulerAngles(delta)));
	}

	Transform& Transform::add_scale(const Vector3f& delta)
	{
		m_scale *= delta;
		m_is_dirty = true;
		return *this;
	}

	Transform& Transform::look_at(const Vector3f& position, const Vector3f& up)
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

	Vector3f Transform::vector_of(const Vector3f& dir) const
	{
		return glm::normalize(glm::mat3_cast(Quaternion(glm::radians(m_rotation))) * dir);
	}

	Vector3f Transform::forward_vector() const
	{
		return vector_of(Constants::forward_vector);
	}

	Vector3f Transform::right_vector() const
	{
		return vector_of(Constants::right_vector);
	}

	Vector3f Transform::up_vector() const
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

	bool Transform::serialize(Archive& ar)
	{
		return ar.serialize(m_location, m_scale, m_rotation);
	}

	static Transform& op_assign(Transform* _this, const Transform& obj)
	{
		(*_this) = obj;
		return *_this;
	}

	static void on_init()
	{
		ScriptClassRegistrar::ValueInfo info = ScriptClassRegistrar::ValueInfo::from<Transform>();
		info.pod                             = true;

		ScriptClassRegistrar registrar = ScriptClassRegistrar::value_class("Engine::Transform", sizeof(Transform), info);

		registrar.behave(ScriptClassBehave::Construct, "void f()", ScriptClassRegistrar::constructor<Transform>,
		                 ScriptCallConv::CDeclObjFirst);
		registrar.behave(ScriptClassBehave::Construct, "void f(const Engine::Transform&)",
		                 ScriptClassRegistrar::constructor<Transform, const Transform&>, ScriptCallConv::CDeclObjFirst);
		registrar.behave(ScriptClassBehave::Destruct, "void f()", ScriptClassRegistrar::destructor<Transform>,
		                 ScriptCallConv::CDeclObjFirst);

		registrar.method("Engine::Transform& opAssign(const Engine::Transform&)", op_assign, ScriptCallConv::CDeclObjFirst);
		registrar.method("const Engine::Matrix4f& matrix() const final", &Transform::matrix);
		registrar.method("Engine::Matrix4f translation_matrix() const final", &Transform::translation_matrix);
		registrar.method("Engine::Matrix4f rotation_matrix() const final", &Transform::rotation_matrix);
		registrar.method("Engine::Matrix4f scale_matrix() const final", &Transform::scale_matrix);
		registrar.method("const Engine::Vector3f& location() const final", method_of<const Vector3f&>(&Transform::location));
		registrar.method("const Engine::Vector3f& rotation() const final", method_of<const Vector3f&>(&Transform::rotation));
		registrar.method("const Engine::Vector3f& scale() const final", method_of<const Vector3f&>(&Transform::scale));
		registrar.method("const Engine::Quaternion& quaternion() const final", method_of<Quaternion>(&Transform::quaternion));

		Transform::static_reflection()->script_type_info = registrar.type_info();
	}

	static ReflectionInitializeController init(on_init);
}// namespace Engine
