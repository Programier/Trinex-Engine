#include <Core/archive.hpp>
#include <Core/buffer_manager.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/math/math.hpp>
#include <Core/reflection/property.hpp>
#include <Core/reflection/struct.hpp>
#include <Core/string_functions.hpp>
#include <Core/transform.hpp>
#include <Engine/ActorComponents/scene_component.hpp>
#include <ScriptEngine/registrar.hpp>
#include <glm/gtx/matrix_decompose.hpp>

namespace Engine
{
	trinex_implement_struct(Engine::Transform, 0)
	{
		auto& location = *trinex_refl_prop(location);
		//auto& rotation = *trinex_refl_prop(m_rotation);
		auto& scale = *trinex_refl_prop(scale);

		location.display_name("Location").tooltip("Location component of transform");
		//rotation.display_name("Rotation").tooltip("Rotation component of transform");
		scale.display_name("Scale").tooltip("Scale component of transform");
	}

	Transform::Transform(const Vector3f& location, const Quaternion& rotation, const Vector3f& scale)
	    : location(location), rotation(rotation), scale(scale)
	{}

	Transform::Transform(const Matrix4f& matrix)
	{
		(*this) = matrix;
	}

	Transform& Transform::operator=(const Matrix4f& matrix)
	{
		Vector3f skew;
		Vector4f perspective;
		glm::decompose(matrix, scale, rotation, location, skew, perspective);
		return *this;
	}

	Matrix4f Transform::translation_matrix() const
	{
		return Math::translate(Matrix4f(1.f), location);
	}

	Matrix4f Transform::rotation_matrix() const
	{
		return glm::mat4_cast(rotation);
	}

	Matrix4f Transform::scale_matrix() const
	{
		return Math::scale(Matrix4f(1.f), scale);
	}

	Quaternion Transform::angles_to_quaternion(const Vector3f& angles)
	{
		return Quaternion(glm::radians(angles));
	}

	Vector3f Transform::quaternion_to_angles(const Quaternion& quat)
	{
		return glm::degrees(glm::eulerAngles(quat));
	}

	Transform& Transform::look_at(const Vector3f& position, const Vector3f& up)
	{
		Transform::operator=(glm::inverse(glm::lookAt(location, position, up)) * scale_matrix());
		return *this;
	}

	Transform Transform::operator+(const Transform& other) const
	{
		Transform new_transform = *this;
		new_transform.location += other.location;
		new_transform.rotation *= other.rotation;
		new_transform.scale *= other.scale;
		return new_transform;
	}

	Transform Transform::operator-(const Transform& other) const
	{
		Transform new_transform = *this;
		new_transform.location -= other.location;
		new_transform.rotation *= Math::inverse(other.rotation);
		new_transform.scale /= other.scale;
		return new_transform;
	}


	Transform& Transform::operator+=(const Transform& other)
	{
		location += other.location;
		rotation *= other.rotation;
		scale *= other.scale;
		return *this;
	}

	Transform& Transform::operator-=(const Transform& other)
	{
		location -= other.location;
		rotation *= Math::inverse(other.rotation);
		scale /= other.scale;
		return *this;
	}

	Matrix4f Transform::matrix() const
	{
		Matrix4f matrix = Math::translate(Matrix4f(1.f), location) * rotation_matrix();
		return Math::scale(matrix, scale);
	}

	Vector3f Transform::vector_of(const Vector3f& dir) const
	{
		return glm::normalize(glm::mat3_cast(rotation) * dir);
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
		                       "Rotation: {}, {}, {}, {}\n"
		                       "Scale: {}, {}, {}",                           //
		                       location.x, location.y, location.z,            //
		                       rotation.x, rotation.y, rotation.z, rotation.w,//
		                       scale.x, scale.y, scale.z);
	}

	bool Transform::serialize(Archive& ar)
	{
		return ar.serialize(location, scale, rotation);
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

		registrar.property("Engine::Vector3f location", &Transform::location);
		registrar.property("Engine::Quaternion rotation", &Transform::rotation);
		registrar.property("Engine::Vector3f scale", &Transform::scale);

		registrar.method("Engine::Transform& opAssign(const Engine::Transform&)", op_assign, ScriptCallConv::CDeclObjFirst);
		registrar.method("const Engine::Matrix4f& matrix() const final", &Transform::matrix);
		registrar.method("Engine::Matrix4f translation_matrix() const final", &Transform::translation_matrix);
		registrar.method("Engine::Matrix4f rotation_matrix() const final", &Transform::rotation_matrix);
		registrar.method("Engine::Matrix4f scale_matrix() const final", &Transform::scale_matrix);

		Transform::static_reflection()->script_type_info = registrar.type_info();
	}

	static ReflectionInitializeController init(on_init);
}// namespace Engine
