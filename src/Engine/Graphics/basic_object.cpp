#include <BasicFunctional/basic_functional.hpp>
#include <Graphics/basic_object.hpp>
#include <glm/ext.hpp>

namespace Engine
{
    const glm::mat4& ModelMatrixObject::model() const
    {
        return _M_model.get();
    }

    void ModelMatrixObject::link_to(const ModelMatrixObject& object)
    {
        _M_model = object._M_model;
    }


    //      TRANSLATE
    const glm::vec3& TranslateObject::position() const
    {
        return _M_position.get();
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
            _M_position.get() += result_move;
        else
        {
            result_move = (-_M_position.get()) + move_vector;
            _M_position = move_vector;
        }

        _M_model = glm::translate(_M_model.get(), result_move);
        return *this;
    }

    TranslateObject& TranslateObject::move(const float& distance, const glm::vec3& axis, const bool& add_value)
    {
        return move(axis * distance, OX, OY, OZ, add_value);
    }

    void TranslateObject::link_to(const TranslateObject& object)
    {
        _M_position = object._M_position;
    }

    //          SCALE
    const glm::vec3& ScaleObject::scale() const
    {
        return _M_scale.get();
    }

    ScaleObject& ScaleObject::scale(const glm::vec3& sc, const bool& add_values)
    {
        glm::vec3 new_sc;
        if (add_values)
        {
            new_sc = sc;
            _M_scale.get() *= sc;
        }
        else
        {
            new_sc = (1.f / _M_scale.get()) * sc;
            _M_scale = sc;
        }
        _M_model = glm::scale(_M_model.get(), new_sc);
        return *this;
    }

    ScaleObject& ScaleObject::scale(const float& x, const float& y, const float& z, const bool& add_values)
    {
        return scale({x, y, z}, add_values);
    }

    void ScaleObject::link_to(const ScaleObject& object)
    {
        _M_scale = object._M_scale;
    }

    //      ROTATION
    const glm::vec3& RotateObject::euler_angles() const
    {
        return _M_euler_angles.get();
    }

    RotateObject& RotateObject::rotate(const float& x, const float& y, const float& z, const bool& add_values)
    {
        return rotate({x, y, z}, add_values);
    }

    void RotateObject::update_model(const glm::quat& q, const bool& add_values)
    {
        if (add_values)
            _M_quaternion.get() *= q;
        else
        {
            _M_quaternion.get().w = -_M_quaternion.get().w;
            _M_model.get() *= glm::mat4_cast(_M_quaternion.get());
            _M_quaternion = q;
        }

        _M_model.get() *= glm::mat4_cast(q);
        _M_euler_angles = glm::eulerAngles(_M_quaternion.get());

        _M_front = glm::normalize(-OZ * _M_quaternion.get());
        _M_up = glm::normalize(OY * _M_quaternion.get());
        _M_right = glm::normalize(OX * _M_quaternion.get());
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
        return _M_quaternion.get();
    }

    const glm::vec3& RotateObject::front_vector() const
    {
        return _M_front.get();
    }

    const glm::vec3& RotateObject::right_vector() const
    {
        return _M_right.get();
    }

    const glm::vec3& RotateObject::up_vector() const
    {
        return _M_up.get();
    }

    void RotateObject::link_to(const RotateObject& object)
    {
        _M_euler_angles = object._M_euler_angles;
        _M_front = object._M_front;
        _M_quaternion = object._M_quaternion;
        _M_up = object._M_up;
        _M_right = object._M_right;
    }

}// namespace Engine
