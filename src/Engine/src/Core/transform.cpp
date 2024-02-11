#include <Core/archive.hpp>
#include <Core/buffer_manager.hpp>
#include <Core/class.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/string_functions.hpp>
#include <Core/transform.hpp>
#include <Engine/ActorComponents/scene_component.hpp>
#include <ScriptEngine/registrar.hpp>
#include <glm/gtx/matrix_decompose.hpp>

namespace Engine
{
    Matrix4f Transform::matrix() const
    {
        Matrix4f transformation = Matrix4f(1.0f);
        transformation          = glm::translate(transformation, location);
        transformation          = transformation * glm::mat4_cast(rotation);
        transformation          = glm::scale(transformation, scale);
        return transformation;
    }


    Transform& Transform::update(class SceneComponent* scene_component, bool is_parent)
    {
        if (!is_parent)
            scene_component = scene_component->parent();

        if (scene_component)
        {
            local_to_world = scene_component->transform.local_to_world * matrix();
        }
        else
        {
            local_to_world = matrix();
        }
        return *this;
    }

    Transform& Transform::update()
    {
        local_to_world = matrix();
        return *this;
    }

    Matrix4f Transform::world_to_local()
    {
        return glm::inverse(local_to_world);
    }

    Vector3D Transform::forward_vector(bool global) const
    {
        static const Vector3D forward = {0.0f, 0.0f, -1.f};
        if (global)
            return glm::normalize(Matrix3f(local_to_world) * forward);
        return glm::normalize(glm::mat3_cast(rotation) * forward);
    }

    Vector3D Transform::right_vector(bool global) const
    {
        static const Vector3D right = {1.0f, 0.0f, 0.f};
        if (global)
            return glm::normalize(Matrix3f(local_to_world) * right);
        return glm::normalize(glm::mat3_cast(rotation) * right);
    }

    Vector3D Transform::up_vector(bool global) const
    {
        static const Vector3D up = {0.0f, 1.0f, 0.f};
        if (global)
            return glm::normalize(Matrix3f(local_to_world) * up);
        return glm::normalize(glm::mat3_cast(rotation) * up);
    }

    Vector3D Transform::global_location() const
    {
        return local_to_world * Vector4D(0.0, 0.0, 0.0, 1.f);
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
        ar & t.location;
        ar & t.scale;
        ar & t.rotation;
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
                         ScriptClassRegistrar::constructor<Transform, const Transform&>, ScriptCallConv::CDECL_OBJFIRST);
        registrar.behave(ScriptClassBehave::Destruct, "void f()", ScriptClassRegistrar::destructor<Transform>,
                         ScriptCallConv::CDECL_OBJFIRST);

        registrar.opfunc("Engine::Transform& opAssign(const Engine::Transform& in)", op_assign, ScriptCallConv::CDECL_OBJFIRST);
    }

    static ScriptEngineInitializeController init(on_init, "Bind Engine::Transform",
                                                 {"Bind Engine::Matrix", "Bind Engine::Vector", "Bind Engine::Quaternion"});
}// namespace Engine
