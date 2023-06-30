#include <Core/buffer_manager.hpp>
#include <Core/class.hpp>
#include <Core/string_functions.hpp>
#include <Core/transform.hpp>
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


    Transform& Transform::move(const Point1D& x, const Point1D& y, const Point1D& z, bool add_values)
    {
        return move({x, y, z}, Constants::OX, Constants::OY, Constants::OZ, add_values);
    }

    Transform& Transform::move(const Vector3D& move_vector, bool add_values)
    {
        return move(move_vector, Constants::OX, Constants::OY, Constants::OZ, add_values);
    }

    Transform& Transform::move(const Point1D& x, const Point1D& y, const Point1D& z, const Vector3D& right,
                               const Vector3D& up, const Vector3D& front, bool add_values)
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

    Transform& Transform::move(const Distance& distance, const Vector3D& axis, bool add_value)
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

    Transform& Transform::scale(const Scale1D& x, const Scale1D& y, const Scale1D& z, bool add_values)
    {
        return scale({x, y, z}, add_values);
    }

    EulerAngle3D Transform::euler_angles() const
    {
        return glm::eulerAngles(_M_quaternion);
    }

    Transform& Transform::rotate(const EulerAngle1D& x, const EulerAngle1D& y, const EulerAngle1D& z, bool add_values)
    {
        return rotate({x, y, z}, add_values);
    }

    Transform& Transform::rotate(const EulerAngle3D& r, bool add_values)
    {
        return rotate(Quaternion(r), add_values);
    }

    Transform& Transform::rotate(const EulerAngle1D& angle, const Vector3D& axis, bool add_values)
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

    static void on_init()
    {
        Lua::Class<Transform> transform_class = Lua::Interpretter::lua_class_of<Transform>("Engine::Transform");

        transform_class.set("matrix", &Transform::matrix);
        transform_class.set("is_modified", &Transform::is_modified);

        // Move methods
        Transform& (Transform::*move1)(const Point1D&, const Point1D&, const Point1D&, bool) = &Transform::move;
        Transform& (Transform::*move2)(const Vector3D&, bool)                                = &Transform::move;
        Transform& (Transform::*move3)(const Point1D&, const Point1D&, const Point1D&, const Vector3D&, const Vector3D&,
                                       const Vector3D&, bool)                                = &Transform::move;

        Transform& (Transform::*move4)(const Vector3D&, const Vector3D&, const Vector3D&, const Vector3D&, bool) =
                &Transform::move;
        Transform& (Transform::*move5)(const Distance&, const Vector3D&, bool) = &Transform::move;

        transform_class.set("move", Lua::overload(move1, move2, move3, move4, move5));
        transform_class.set("position", &Transform::position);

        // Scale methods

        const Scale3D& (Transform::*scale1)() const                                           = &Transform::scale;
        Transform& (Transform::*scale2)(const Scale3D&, bool)                                 = &Transform::scale;
        Transform& (Transform::*scale3)(const Scale1D&, const Scale1D&, const Scale1D&, bool) = &Transform::scale;

        transform_class.set("scale", Lua::overload(scale1, scale2, scale3));

        // Rotation methods

        transform_class.set("euler_angles", &Transform::euler_angles);
        transform_class.set("quaternion", &Transform::quaternion);
        transform_class.set("front_vector", &Transform::front_vector);
        transform_class.set("right_vector", &Transform::right_vector);
        transform_class.set("up_vector", &Transform::up_vector);

        Transform& (Transform::*rotate1)(const EulerAngle1D&, const EulerAngle1D&, const EulerAngle1D&, bool) =
                &Transform::rotate;
        Transform& (Transform::*rotate2)(const EulerAngle3D&, bool)                  = &Transform::rotate;
        Transform& (Transform::*rotate3)(const EulerAngle1D&, const Vector3D&, bool) = &Transform::rotate;
        Transform& (Transform::*rotate4)(const Quaternion&, bool)                    = &Transform::rotate;

        transform_class.set("rotate", Lua::overload(rotate1, rotate2, rotate3, rotate4));

        transform_class.set("as_string", &Transform::as_string);
        transform_class.set(Lua::meta_function::to_string, &Transform::as_string);
    }

    static InitializeController init(on_init);
}// namespace Engine
