#include <Core/buffer_manager.hpp>
#include <Core/class.hpp>
#include <Core/string_functions.hpp>
#include <Core/transform.hpp>
#include <ScriptEngine/registrar.hpp>
#include <glm/gtx/matrix_decompose.hpp>

namespace Engine
{
    const Matrix4f& Transform::matrix() const
    {
        if (_M_is_modified)
            update_data();
        return _M_matrix;
    }

    bool Transform::is_modified() const
    {
        return static_cast<bool>(_M_is_modified);
    }

    void Transform::update_data() const
    {
        new (&_M_matrix) Matrix4f(1.0f);

        _M_matrix *= glm::mat4_cast(_M_quaternion);
        _M_matrix      = glm::scale(_M_matrix, _M_scale);
        _M_matrix      = glm::translate(_M_matrix, _M_position);
        _M_is_modified = 0;
    }


#define APPLY_TRANSFORM(condition, operation, component, code)                                                         \
    _M_is_modified = 1;                                                                                                \
    if (condition)                                                                                                     \
    {                                                                                                                  \
        component operation code;                                                                                      \
    }                                                                                                                  \
    else                                                                                                               \
    {                                                                                                                  \
        component = code;                                                                                              \
    }


    Transform& Transform::move(Point1D x, Point1D y, Point1D z, bool add_values)
    {
        return move({x, y, z}, Constants::OX, Constants::OY, Constants::OZ, add_values);
    }

    Transform& Transform::move(const Vector3D& move_vector, bool add_values)
    {
        return move(move_vector, Constants::OX, Constants::OY, Constants::OZ, add_values);
    }

    Transform& Transform::move(Point1D x, Point1D y, Point1D z, const Vector3D& right, const Vector3D& up,
                               const Vector3D& front, bool add_values)
    {
        return move({x, y, z}, right, up, front, add_values);
    }

    Transform& Transform::move(const Vector3D& move_vector, const Vector3D& x_axis, const Vector3D& y_axis,
                               const Vector3D& z_axis, bool add_values)
    {
        APPLY_TRANSFORM(add_values, +=, _M_position,
                        (x_axis * move_vector.x) + (y_axis * move_vector.y) + (z_axis * move_vector.z));
        return *this;
    }

    Transform& Transform::move(Distance distance, const Vector3D& axis, bool add_value)
    {
        return move(axis * distance, Constants::OX, Constants::OY, Constants::OZ, add_value);
    }

    const Point3D& Transform::position() const
    {
        return _M_position;
    }

    const Scale3D& Transform::scale() const
    {
        return _M_scale;
    }

    Transform& Transform::scale(const Scale3D& sc, bool add_values)
    {
        APPLY_TRANSFORM(add_values, *=, _M_scale, sc);
        return *this;
    }

    Transform& Transform::scale(float x, float y, float z, bool add_values)
    {
        return scale({x, y, z}, add_values);
    }

    EulerAngle3D Transform::euler_angles() const
    {
        return glm::eulerAngles(_M_quaternion);
    }

    Transform& Transform::rotate(float x, float y, float z, bool add_values)
    {
        return rotate({x, y, z}, add_values);
    }

    Transform& Transform::rotate(const EulerAngle3D& r, bool add_values)
    {
        return rotate(Quaternion(r), add_values);
    }

    Transform& Transform::rotate(float angle, const Vector3D& axis, bool add_values)
    {
        glm::quat q = glm::rotate(glm::quat(Constants::zero_vector), angle, axis);
        return rotate(q, add_values);
    }

    Transform& Transform::rotate(const Quaternion& q, bool add_values)
    {
        _M_is_modified = 1;
        _M_quaternion  = add_values ? q * _M_quaternion : q;
        return *this;
    }

    const Quaternion& Transform::quaternion() const
    {
        return _M_quaternion;
    }

    Vector3D Transform::front_vector() const
    {
        if (_M_revert_front_vector)
            return -glm::normalize(_M_quaternion * Constants::OZ);
        return glm::normalize(_M_quaternion * Constants::OZ);
    }

    Vector3D Transform::right_vector() const
    {
        return glm::normalize(_M_quaternion * Constants::OX);
    }

    Vector3D Transform::up_vector() const
    {
        return glm::normalize(_M_quaternion * Constants::OY);
    }

    String Transform::as_string() const
    {
        return Strings::format("Position: {}\n"
                               "Scale:    {}\n"
                               "Rotation: {}",
                               _M_position, _M_scale, euler_angles());
    }

    bool operator&(Archive& ar, Transform& t)
    {
        ar& t._M_position.x;
        ar& t._M_position.y;
        ar& t._M_position.z;

        ar& t._M_scale.x;
        ar& t._M_scale.y;
        ar& t._M_scale.z;

        ar& t._M_quaternion.x;
        ar& t._M_quaternion.y;
        ar& t._M_quaternion.z;
        ar& t._M_quaternion.w;

        if (ar.is_reading())
        {
            t._M_is_modified = 1;
        }

        return static_cast<bool>(ar);
    }


    static Transform& op_assign(Transform* _this, const Transform& obj)
    {
        (*_this) = obj;
        return *_this;
    }

    static void on_init()
    {
        Transform& (Transform::*move1)(Point1D, Point1D, Point1D, bool) = &Transform::move;
        Transform& (Transform::*move2)(const Vector3D&, bool)           = &Transform::move;
        Transform& (Transform::*move3)(Point1D, Point1D, Point1D, const Vector3D&, const Vector3D&, const Vector3D&,
                                       bool)                            = &Transform::move;
        Transform& (Transform::*move4)(const Vector3D&, const Vector3D&, const Vector3D&, const Vector3D&, bool) =
                &Transform::move;
        Transform& (Transform::*move5)(Distance, const Vector3D&, bool) = &Transform::move;
        const Scale3D& (Transform::*scale1)() const                     = &Transform::scale;
        Transform& (Transform::*scale2)(const Scale3D&, bool)           = &Transform::scale;
        Transform& (Transform::*scale3)(float, float, float, bool)      = &Transform::scale;
        Transform& (Transform::*rotate1)(float, float, float, bool)     = &Transform::rotate;
        Transform& (Transform::*rotate2)(const EulerAngle3D&, bool)     = &Transform::rotate;
        Transform& (Transform::*rotate3)(float, const Vector3D&, bool)  = &Transform::rotate;
        Transform& (Transform::*rotate4)(const Quaternion&, bool)       = &Transform::rotate;


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

        registrar.method("const Engine::Matrix4f& matrix() const", &Transform::matrix, ScriptCallConv::THISCALL);
        registrar.method("bool is_modified() const", &Transform::is_modified, ScriptCallConv::THISCALL);
        registrar.method("Transform& move(float, float, float, bool = true) const", move1, ScriptCallConv::THISCALL);
        registrar.method("Transform& move(const Vector3D&, bool = true) const", move2, ScriptCallConv::THISCALL);
        registrar.method("Transform& move(float, float, float,  const Vector3D&, const Vector3D&, const Vector3D&, "
                         "bool = true) const",
                         move3, ScriptCallConv::THISCALL);
        registrar.method(
                "Transform& move(const Vector3D&, const Vector3D&, const Vector3D&, const Vector3D&, bool = true)",
                move4, ScriptCallConv::THISCALL);
        registrar.method("Transform& move(float, const Vector3D&, bool = true)", move5, ScriptCallConv::THISCALL);
        registrar.method("const Vector3D& position() const", &Transform::position, ScriptCallConv::THISCALL);
        registrar.method("const Vector3D& scale() const", scale1, ScriptCallConv::THISCALL);
        registrar.method("Vector3D up_vector() const", &Transform::front_vector, ScriptCallConv::THISCALL);
        registrar.method("Vector3D right_vector() const", &Transform::right_vector, ScriptCallConv::THISCALL);
        registrar.method("Vector3D front_vector() const", &Transform::up_vector, ScriptCallConv::THISCALL);
        registrar.method("Vector3D euler_angles() const", &Transform::euler_angles, ScriptCallConv::THISCALL);
        registrar.method("string as_string() const", &Transform::as_string, ScriptCallConv::THISCALL);
        registrar.method("const Quaternion& quaternion() const", &Transform::quaternion, ScriptCallConv::THISCALL);


        registrar.method("Transform& scale(const Vector3D& , bool = true)", scale2, ScriptCallConv::THISCALL);
        registrar.method("Transform& scale(float, float, float, bool = true)", scale3, ScriptCallConv::THISCALL);
        registrar.method("Transform& rotate(float, float, float, bool = true)", rotate1, ScriptCallConv::THISCALL);
        registrar.method("Transform& rotate(const Vector3D&, bool = true)", rotate2, ScriptCallConv::THISCALL);
        registrar.method("Transform& rotate(float, const Vector3D&, bool = true)", rotate3, ScriptCallConv::THISCALL);
        registrar.method("Transform& rotate(const Quaternion&, bool = true)", rotate4, ScriptCallConv::THISCALL);
    }

    static InitializeController init(on_init, "Bind Engine::Transform", {"Bind Engine::Matrix", "Bind Engine::Vector"});
}// namespace Engine
