#include <Core/buffer_manager.hpp>
#include <Core/class.hpp>
#include <Core/string_functions.hpp>
#include <Core/transform.hpp>
#include <ScriptEngine/registrar.hpp>
#include <glm/gtx/matrix_decompose.hpp>

namespace Engine
{
    Matrix4f Transform::local_to_world() const
    {
        Matrix4f transformation = Matrix4f(1.0f);
        transformation          = glm::translate(transformation, position);
        transformation          = transformation * glm::mat4_cast(rotation);
        transformation          = glm::scale(transformation, scale);
        return transformation;
    }

    Matrix4f Transform::world_to_local()
    {
        return glm::inverse(local_to_world());
    }

    Vector3D Transform::forward_vector() const
    {
        return glm::normalize(glm::mat3_cast(rotation) * Vector3D(0.0f, 0.0f, -1.0f));
    }

    Vector3D Transform::right_vector() const
    {
        return glm::normalize(glm::mat3_cast(rotation) * Vector3D(1.0f, 0.0f, 0.0f));
    }

    Vector3D Transform::up_vector() const
    {
        return glm::normalize(glm::mat3_cast(rotation) * Vector3D(0.0f, 1.0f, 0.0f));
    }

    String Transform::as_string() const
    {
        return Strings::format("Position: {}\n"
                               "Scale:    {}\n"
                               "Rotation: {}",
                               position, scale, rotation);
    }


    Quaternion Transform::calc_rotation(const Vector3D& axis, float angle)
    {
        return glm::angleAxis(angle, axis);
    }

    Quaternion Transform::calc_rotation(const Vector3D& angles)
    {
        return Quaternion(angles);
    }

    bool operator&(Archive& ar, Transform& t)
    {
        ar& t.position;
        ar& t.scale;
        ar& t.rotation;
        return static_cast<bool>(ar);
    }


    static Transform& op_assign(Transform* _this, const Transform& obj)
    {
        (*_this) = obj;
        return *_this;
    }

    static void on_init()
    {
        ScriptClassRegistrar registrar("Engine::Transform",
                                       ScriptClassRegistrar::create_type_info<Transform>(ScriptClassRegistrar::Value));

        registrar.behave(ScriptClassBehave::Construct, "void f()", ScriptClassRegistrar::constructor<Transform>,
                         ScriptCallConv::CDECL_OBJFIRST);
        registrar.behave(ScriptClassBehave::Construct, "void f(const Engine::Transform& in)",
                         ScriptClassRegistrar::constructor<Transform, const Transform&>,
                         ScriptCallConv::CDECL_OBJFIRST);
        registrar.behave(ScriptClassBehave::Destruct, "void f()", ScriptClassRegistrar::destructor<Transform>,
                         ScriptCallConv::CDECL_OBJFIRST);

        registrar.opfunc("Engine::Transform& opAssign(const Engine::Transform& in)", op_assign,
                         ScriptCallConv::CDECL_OBJFIRST);
    }

    static InitializeController init(on_init, "Bind Engine::Transform",
                                     {"Bind Engine::Matrix", "Bind Engine::Vector", "Bind Engine::Quaternion"});
}// namespace Engine
