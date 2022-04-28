#include <BasicFunctional/basic_functional.hpp>
#include <Graphics/basic_object.hpp>
#include <algorithm>
#include <glm/ext.hpp>


#define TRANSLATE_INDEX 0
#define ROTATE_INDEX 1
#define SCALE_INDEX 2


#define get_model(index) _M_models[index].is_null() ? identity_matrix : _M_models[index].get();

namespace Engine
{

    const glm::mat4& ModelMatrixObject::rotation_matrix() const
    {
        return get_model(ROTATE_INDEX);
    }

    const glm::mat4& ModelMatrixObject::scale_matrix() const
    {
        return get_model(SCALE_INDEX);
    }

    const glm::mat4& ModelMatrixObject::translate_matrix() const
    {
        return get_model(TRANSLATE_INDEX);
    }

    glm::mat4& ModelMatrixObject::_M_rotation_matrix()
    {
        return _M_models[1].get();
    }

    glm::mat4& ModelMatrixObject::_M_scale_matrix()
    {
        return _M_models[2].get();
    }

    glm::mat4& ModelMatrixObject::_M_translate_matrix()
    {
        return _M_models[0].get();
    }

    glm::mat4 ModelMatrixObject::model() const
    {
        glm::mat4 result(1.0f);
        for (auto& ell : _M_models)
        {
            if (!ell.is_null())
            {
                result *= ell.get();
            }
        }
        return result;
    }


    //      TRANSLATE

    TranslateObject::TranslateObject()
    {
        _M_models[TRANSLATE_INDEX] = identity_matrix;
    }

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

    TranslateObject& TranslateObject::move(const float& x, const float& y, const float& z, const glm::vec3& right, const glm::vec3& up,
                                           const glm::vec3& front, const bool& add_values)
    {
        return move({x, y, z}, right, up, front, add_values);
    }

    TranslateObject& TranslateObject::move(const glm::vec3 move_vector, const glm::vec3& right, const glm::vec3& up,
                                           const glm::vec3& front, const bool& add_values)
    {
        glm::vec3 result_move = (right * move_vector.x) + (up * move_vector.y) + (front * move_vector.z);

        if (add_values)
            this->_M_position.get() += result_move;
        else
        {
            result_move = (-this->_M_position.get()) + move_vector;
            this->_M_position = move_vector;
        }

        this->_M_translate_matrix() = glm::translate(this->_M_translate_matrix(), result_move);
        return *this;
    }

    TranslateObject& TranslateObject::move(const float& distance, const glm::vec3& axis, const bool& add_value)
    {
        return move(axis * distance, OX, OY, OZ, add_value);
    }

    TranslateObject& TranslateObject::link_to(TranslateObject& obj)
    {
        _M_models[TRANSLATE_INDEX] = obj._M_models[TRANSLATE_INDEX];
        _M_position = obj._M_position;
        return *this;
    }


    TranslateObject::~TranslateObject()
    {}

    //          SCALE
    ScaleObject::ScaleObject()
    {
        _M_models[SCALE_INDEX] = identity_matrix;
    }

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
        _M_scale_matrix() = glm::scale(_M_scale_matrix(), new_sc);
        return *this;
    }

    ScaleObject& ScaleObject::scale(const float& x, const float& y, const float& z, const bool& add_values)
    {
        return scale({x, y, z}, add_values);
    }

    ScaleObject& ScaleObject::link_to(ScaleObject& obj)
    {
        _M_scale_matrix() = obj._M_scale_matrix();
        _M_scale = obj._M_scale;
        return *this;
    }

    //      ROTATION
    RotateObject::RotateObject()
    {
        _M_models[ROTATE_INDEX] = identity_matrix;
    }

    const glm::vec3& RotateObject::euler_angles() const
    {
        return _M_euler_angles.get();
    }

    RotateObject& RotateObject::rotate(const float& x, const float& y, const float& z, const bool& add_values)
    {
        return rotate(glm::quat(glm::vec3(x, y, z)), add_values);
    }

    void RotateObject::update_model(const glm::quat& q, const bool& add_values)
    {
        auto& quat = _M_quaternion.get();
        auto& matrix = _M_rotation_matrix();
        if (add_values)
            _M_quaternion = q * quat;
        else
        {
            quat.w = -quat.w;
            matrix = identity_matrix;
            quat = q;
        }

        matrix = glm::mat4_cast(q) * matrix;
        _M_euler_angles.get() = glm::eulerAngles(quat);

        _M_front = glm::normalize(quat * OZ);
        _M_up = glm::normalize(quat * OY);
        _M_right = glm::normalize(quat * OX);
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

    RotateObject& RotateObject::rotate(const glm::quat& q, const bool& add_values)
    {
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

    RotateObject& RotateObject::link_to(RotateObject& obj)
    {
        _M_quaternion = obj._M_quaternion;
        _M_euler_angles = obj._M_euler_angles;
        _M_front = obj._M_front;
        _M_right = obj._M_right;
        _M_up = obj._M_up;
        _M_models[ROTATE_INDEX] = obj._M_models[ROTATE_INDEX];
        return *this;
    }


}// namespace Engine
