#include <BasicFunctional/basic_functional.hpp>
#include <Graphics/basic_object.hpp>
#include <glm/ext.hpp>

namespace Engine
{
    const glm::mat4& ModelMatrixObject::model() const
    {
        return _M_model;
    }


    //      TRANSLATE
    const glm::vec3& TranslateObject::position() const
    {
        return _M_position;
    }

    TranslateObject& TranslateObject::move(const float& x, const float& y, const float& z, const bool& add_values)
    {
        return move({x, y, z}, OX, OY, OZ, add_values);
    }

    TranslateObject& TranslateObject::move(const glm::vec3& move_vector, const bool& add_values)
    {
        return move(move_vector, OX, OY, OZ, add_values);
    }

    TranslateObject& TranslateObject::move(const float& x, const float& y, const float& z, const glm::vec3& right,
                                           const glm::vec3& up, const glm::vec3& front, const bool& add_values)
    {
        return move({x, y, z}, right, up, front, add_values);
    }

    TranslateObject& TranslateObject::move(const glm::vec3 move_vector, const glm::vec3& right, const glm::vec3& up,
                                           const glm::vec3& front, const bool& add_values)
    {
        glm::vec3 result_move = (right * move_vector.x) + (up * move_vector.y) + (front * move_vector.z);

        if (add_values)
            _M_position += result_move;
        else
        {
            result_move = (-_M_position) + move_vector;
            _M_position = move_vector;
        }

        _M_model = glm::translate(_M_model, result_move);
        return *this;
    }

    TranslateObject& TranslateObject::move(const float& distance, const glm::vec3& axis, const bool& add_value)
    {
        return move(axis * distance, OX, OY, OZ, add_value);
    }

    //          SCALE
    const glm::vec3& ScaleObject::scale() const
    {
        return _M_scale;
    }

    ScaleObject& ScaleObject::scale(const glm::vec3& sc, const bool& add_values)
    {
        glm::vec3 new_sc;
        if (add_values)
        {
            new_sc = sc;
            _M_scale *= sc;
        }
        else
        {
            new_sc = (1.f / _M_scale) * sc;
            _M_scale = sc;
        }
        _M_model = glm::scale(_M_model, new_sc);
        return *this;
    }

    ScaleObject& ScaleObject::scale(const float& x, const float& y, const float& z, const bool& add_values)
    {
        return scale({x, y, z}, add_values);
    }

    //      ROTATION
    const glm::vec3& RotateObject::euler_angles() const
    {
        return _M_euler_angles;
    }

    RotateObject& RotateObject::rotate(const float& x, const float& y, const float& z, const bool& add_values)
    {
        return rotate({x, y, z}, add_values);
    }

    void RotateObject::update_model(const glm::quat& q, const bool& add_values)
    {
        if (add_values)
            _M_quaternion *= q;
        else
        {
            _M_quaternion.w = -_M_quaternion.w;
            _M_model *= glm::mat4_cast(_M_quaternion);
            _M_quaternion = q;
        }

        _M_model *= glm::mat4_cast(q);
        _M_euler_angles = glm::eulerAngles(_M_quaternion);

        _M_front = glm::normalize(-OZ * _M_quaternion);
        _M_up = glm::normalize(OY * _M_quaternion);
        _M_right = glm::normalize(OX * _M_quaternion);
    }

    RotateObject& RotateObject::rotate(const glm::vec3& r, const bool& add_values)
    {
        glm::quat q(r);
        update_model(q, add_values);
        return *this;
    }

    RotateObject& RotateObject::rotate(const float& angle, const glm::vec3& axis, const bool& add_values)
    {
        glm::quat q = glm::rotate(glm::quat(glm::vec3(0.f, 0.f, 0.f)), angle, axis);
        update_model(q, add_values);
        return *this;
    }

    const glm::quat& RotateObject::quaternion() const
    {
        return _M_quaternion;
    }

    const glm::vec3& RotateObject::front_vector() const
    {
        return _M_front;
    }

    const glm::vec3& RotateObject::right_vector() const
    {
        return _M_right;
    }

    const glm::vec3& RotateObject::up_vector() const
    {
        return _M_up;
    }
}// namespace Engine
