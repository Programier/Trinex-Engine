#include <Graphics/basic_object.hpp>
#include <algorithm>
#include <engine.hpp>
#include <glm/ext.hpp>
#include <numeric>

#define TRANSLATE_INDEX 0
#define ROTATE_INDEX 1
#define SCALE_INDEX 2


#define get_model(index) _M_models[index].is_null() ? identity_matrix : _M_models[index].get();

namespace Engine
{

    const glm::mat4& ModelMatrix::rotation_matrix() const
    {
        return get_model(ROTATE_INDEX);
    }

    const glm::mat4& ModelMatrix::scale_matrix() const
    {
        return get_model(SCALE_INDEX);
    }

    const glm::mat4& ModelMatrix::translate_matrix() const
    {
        return get_model(TRANSLATE_INDEX);
    }

    glm::mat4& ModelMatrix::_M_rotation_matrix()
    {
        return _M_models[1].get();
    }

    glm::mat4& ModelMatrix::_M_scale_matrix()
    {
        return _M_models[2].get();
    }

    glm::mat4& ModelMatrix::_M_translate_matrix()
    {
        return _M_models[0].get();
    }

    glm::mat4 ModelMatrix::model() const
    {
        glm::mat4 result = _M_shift_models[3].get();
        for (auto& ell : _M_models) result *= ell.get();
        return result;
    }

    ModelMatrix& ModelMatrix::recalculate_shift()
    {
        _M_shift_models[3] = identity_matrix;
        for (std::size_t i = 0; i < 3; i++) _M_shift_models[3].get() *= _M_shift_models[i].get();
        return *this;
    }


    //      TRANSLATE

    Point3D Translate::position() const
    {
        return _M_position.get() + _M_shift_position;
    }

    Translate& Translate::move(const Point1D& x, const Point1D& y, const Point1D& z, const bool& add_values)
    {
        return move({x, y, z}, OX, OY, OZ, add_values);
    }

    Translate& Translate::move(const Vector3D& move_vector, const bool& add_values)
    {
        return move(move_vector, OX, OY, OZ, add_values);
    }

    Translate& Translate::move(const Point1D& x, const Point1D& y, const Point1D& z, const Vector3D& right,
                               const Vector3D& up, const Vector3D& front, const bool& add_values)
    {
        return move({x, y, z}, right, up, front, add_values);
    }

    Translate& Translate::move(const Vector3D move_vector, const Vector3D& right, const Vector3D& up, const Vector3D& front,
                               const bool& add_values)
    {
        glm::vec3 result_move = (right * move_vector.x) + (up * move_vector.y) + (front * move_vector.z);

        if (add_values)
            this->_M_position.get() += result_move;
        else
        {
            _M_translate_matrix() = identity_matrix;
            this->_M_position = move_vector;
        }

        this->_M_translate_matrix() = glm::translate(this->_M_translate_matrix(), result_move);
        return *this;
    }

    Translate& Translate::move(const Distance& distance, const glm::vec3& axis, const bool& add_value)
    {
        return move(axis * distance, OX, OY, OZ, add_value);
    }

    Translate& Translate::link_to(Translate& obj)
    {
        _M_models[TRANSLATE_INDEX] = obj._M_models[TRANSLATE_INDEX];
        _M_position = obj._M_position;
        return *this;
    }


    Translate::~Translate()
    {}


    //          SCALE

    const Scale3D& Scale::scale() const
    {
        return _M_scale.get();
    }

    Scale& Scale::scale(const Scale3D& sc, const bool& add_values)
    {
        Scale3D new_sc;
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

    Scale& Scale::scale(const Scale1D& x, const Scale1D& y, const Scale1D& z, const bool& add_values)
    {
        return scale({x, y, z}, add_values);
    }

    Scale& Scale::link_to(Scale& obj)
    {
        _M_scale_matrix() = obj._M_scale_matrix();
        _M_scale = obj._M_scale;
        return *this;
    }

    //      ROTATION

    const EulerAngle3D& Rotate::euler_angles() const
    {
        return _M_euler_angles.get();
    }

    Rotate& Rotate::rotate(const EulerAngle1D& x, const EulerAngle1D& y, const EulerAngle1D& z, const bool& add_values)
    {
        return rotate(glm::quat(glm::vec3(x, y, z)), add_values);
    }

    void Rotate::update_model(const Quaternion& q, const bool& add_values)
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

        _M_quaternion.get() = glm::normalize(_M_quaternion.get());
        matrix = glm::mat4_cast(glm::normalize(q)) * matrix;
        _M_euler_angles.get() = glm::eulerAngles(quat);

        _M_front = glm::normalize(quat * OZ);
        _M_up = glm::normalize(quat * OY);
        _M_right = glm::normalize(quat * OX);
    }

    Rotate& Rotate::rotate(const EulerAngle3D& r, const bool& add_values)
    {
        Quaternion q(r);
        update_model(q, add_values);
        return *this;
    }

    Rotate& Rotate::rotate(const EulerAngle1D& angle, const Vector3D& axis, const bool& add_values)
    {
        glm::quat q = glm::rotate(glm::quat(glm::vec3(0.f, 0.f, 0.f)), angle, axis);
        update_model(q, add_values);
        return *this;
    }

    Rotate& Rotate::rotate(const Quaternion& q, const bool& add_values)
    {
        update_model(q, add_values);
        return *this;
    }

    const Quaternion& Rotate::quaternion() const
    {
        return _M_quaternion.get();
    }

    const Vector3D& Rotate::front_vector() const
    {
        return _M_front.get();
    }

    const Vector3D& Rotate::right_vector() const
    {
        return _M_right.get();
    }

    const Vector3D& Rotate::up_vector() const
    {
        return _M_up.get();
    }

    Rotate& Rotate::link_to(Rotate& obj)
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