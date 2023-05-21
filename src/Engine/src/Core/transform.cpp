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

    Transform& Transform::move(const Vector3D move_vector, const Vector3D& x_axis, const Vector3D& y_axis,
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
}// namespace Engine
