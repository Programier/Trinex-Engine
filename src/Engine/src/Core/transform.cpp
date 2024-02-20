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
    static void rotate_x(Matrix4f& transformation, const Vector3D& rotation)
    {
        transformation = glm::rotate(transformation, glm::radians(rotation.x), Constants::OX);
    }

    static void rotate_y(Matrix4f& transformation, const Vector3D& rotation)
    {
        transformation = glm::rotate(transformation, glm::radians(rotation.y), Constants::OY);
    }

    static void rotate_z(Matrix4f& transformation, const Vector3D& rotation)
    {
        transformation = glm::rotate(transformation, glm::radians(rotation.z), Constants::OZ);
    }

    static void rotate_xyz(Matrix4f& transformation, const Vector3D& rotation)
    {
        rotate_x(transformation, rotation);
        rotate_y(transformation, rotation);
        rotate_z(transformation, rotation);
    }

    static void rotate_xzy(Matrix4f& transformation, const Vector3D& rotation)
    {
        rotate_x(transformation, rotation);
        rotate_z(transformation, rotation);
        rotate_y(transformation, rotation);
    }

    static void rotate_yxz(Matrix4f& transformation, const Vector3D& rotation)
    {
        rotate_y(transformation, rotation);
        rotate_x(transformation, rotation);
        rotate_z(transformation, rotation);
    }

    static void rotate_yzx(Matrix4f& transformation, const Vector3D& rotation)
    {
        rotate_y(transformation, rotation);
        rotate_z(transformation, rotation);
        rotate_x(transformation, rotation);
    }

    static void rotate_zxy(Matrix4f& transformation, const Vector3D& rotation)
    {
        rotate_z(transformation, rotation);
        rotate_x(transformation, rotation);
        rotate_y(transformation, rotation);
    }

    static void rotate_zyx(Matrix4f& transformation, const Vector3D& rotation)
    {
        rotate_z(transformation, rotation);
        rotate_y(transformation, rotation);
        rotate_x(transformation, rotation);
    }


    static const Array<void (*)(Matrix4f&, const Vector3D&), 6>& rotation_methods()
    {
        static const Array<void (*)(Matrix4f&, const Vector3D&), 6> methods = {rotate_xyz, rotate_xzy, rotate_yxz,
                                                                               rotate_yzx, rotate_zxy, rotate_zyx};

        return methods;
    }


    Matrix4f Transform::translation_matrix() const
    {
        return glm::translate(Matrix4f(1.f), location);
    }

    Matrix4f Transform::rotation_matrix() const
    {
        Matrix4f result(1.f);
        rotation_methods()[static_cast<int>(rotation_method)](result, rotation);
        return result;
    }

    Matrix4f Transform::scale_matrix() const
    {
        return glm::scale(Matrix4f(1.f), scale);
    }

    Matrix4f Transform::matrix() const
    {
        Matrix4f transformation = Matrix4f(1.0f);
        transformation          = glm::translate(transformation, location);

        rotation_methods()[static_cast<int>(rotation_method)](transformation, rotation);
        transformation = glm::scale(transformation, scale);
        return transformation;
    }


    Transform& Transform::update(class SceneComponent* scene_component)
    {
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

    Matrix4f Transform::world_to_local()
    {
        return glm::inverse(local_to_world);
    }

    Vector3D Transform::vector_of(const Vector3D& dir, bool is_global) const
    {
        if (is_global)
            return glm::normalize(Matrix3f(local_to_world) * dir);

        Matrix4f rotation_matrix(1.f);
        rotation_methods()[static_cast<int>(rotation_method)](rotation_matrix, rotation);
        return glm::normalize(Matrix3f(rotation_matrix) * dir);
    }

    Vector3D Transform::forward_vector(bool global) const
    {
        return vector_of(Constants::forward_vector, global);
    }

    Vector3D Transform::right_vector(bool global) const
    {
        return vector_of(Constants::right_vector, global);
    }

    Vector3D Transform::up_vector(bool global) const
    {
        return vector_of(Constants::up_vector, global);
    }

    Vector3D Transform::global_location() const
    {
        return local_to_world * Vector4D(0.0, 0.0, 0.0, 1.f);
    }

    String Transform::as_string() const
    {
        return Strings::format("Location: {}, {}, {}\n"
                               "Rotation: {}, {}, {}\n"
                               "Scale: {}, {}, {}",               //
                               location.x, location.y, location.z,//
                               rotation.x, rotation.y, rotation.z,//
                               scale.x, scale.y, scale.z);
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
