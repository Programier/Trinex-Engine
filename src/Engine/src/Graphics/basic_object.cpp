#include <Core/engine.hpp>
#include <Graphics/basic_object.hpp>
#include <algorithm>
#include <glm/ext.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <numeric>


namespace Engine
{
    ModelMatrix::ModelMatrix()
    {}

    const Matrix4f& ModelMatrix::model() const
    {
        return _M_model;
    }


    ModelMatrix& ModelMatrix::update_data()
    {
        glm::vec3 skew;
        glm::vec4 perspective;

        glm::decompose(_M_model, _M_scale, _M_quaternion, _M_position, skew, perspective);

        _M_euler_angles = glm::eulerAngles(_M_quaternion);
        update_vectors();
        return *this;
    }

    ModelMatrix& ModelMatrix::update_vectors()
    {
        Quaternion tmp(_M_euler_angles);
        _M_front = glm::normalize(tmp * Constants::OZ);
        _M_up    = glm::normalize(tmp * Constants::OY);
        _M_right = glm::normalize(tmp * Constants::OX);
        return *this;
    }

    ModelMatrix& ModelMatrix::model(const glm::mat4& m)
    {
        on_before_set_model.trigger(this);

        _M_model = m;
        update_data();

        on_set_model.trigger(this);
        return *this;
    }


    //      TRANSLATE


    Translate::Translate()
    {}

    const Point3D& Translate::position() const
    {
        return _M_position;
    }

    Translate& Translate::move(const Point1D& x, const Point1D& y, const Point1D& z, const bool& add_values)
    {
        return move({x, y, z}, Constants::OX, Constants::OY, Constants::OZ, add_values);
    }

    Translate& Translate::move(const Vector3D& move_vector, const bool& add_values)
    {
        return move(move_vector, Constants::OX, Constants::OY, Constants::OZ, add_values);
    }

    Translate& Translate::move(const Point1D& x, const Point1D& y, const Point1D& z, const Vector3D& right,
                               const Vector3D& up, const Vector3D& front, const bool& add_values)
    {
        return move({x, y, z}, right, up, front, add_values);
    }

    Translate& Translate::move(const Vector3D move_vector, const Vector3D& right, const Vector3D& up,
                               const Vector3D& front, const bool& add_values)
    {
        on_before_translate.trigger(this);


        glm::vec3 result_move = (right * move_vector.x) + (up * move_vector.y) + (front * move_vector.z);

        if (add_values)
        {
            this->_M_position += result_move;
            _M_model = glm::translate(_M_model, result_move);
        }
        else
        {
            _M_model    = glm::translate(_M_model, -_M_position + result_move);
            _M_position = move_vector;
        }


        on_translate.trigger(this);

        return *this;
    }

    Translate& Translate::move(const Distance& distance, const glm::vec3& axis, const bool& add_value)
    {
        return move(axis * distance, Constants::OX, Constants::OY, Constants::OZ, add_value);
    }


    //          SCALE
    Scale::Scale()
    {}


    const Scale3D& Scale::scale() const
    {
        return _M_scale;
    }

    Scale& Scale::scale(const Scale3D& sc, const bool& add_values)
    {
        if (sc.x == 0.f || sc.y == 0.f || sc.z == 0.f)
            return *this;

        on_before_scale.trigger(this);


        if (add_values)
        {
            _M_scale *= sc;
        }
        else
        {
            _M_model = glm::scale(_M_model, 1.f / _M_scale);
            _M_scale = sc;
        }

        _M_model = glm::scale(_M_model, sc);

        on_scale.trigger(this);

        return *this;
    }

    Scale& Scale::scale(const Scale1D& x, const Scale1D& y, const Scale1D& z, const bool& add_values)
    {
        return scale({x, y, z}, add_values);
    }


    //      ROTATION

    Rotate::Rotate() = default;


    const EulerAngle3D& Rotate::euler_angles() const
    {
        return _M_euler_angles;
    }

    Rotate& Rotate::rotate(const EulerAngle1D& x, const EulerAngle1D& y, const EulerAngle1D& z, const bool& add_values)
    {
        return rotate(glm::quat(glm::vec3(x, y, z)), add_values);
    }

    void Rotate::update_model(const Quaternion& q, const bool& add_values)
    {
        on_before_rotate.trigger(this);

        auto& quat = _M_quaternion;

        if (add_values)
            _M_quaternion = q * quat;
        else
        {
            quat.w   = -quat.w;
            _M_model = _M_model * glm::mat4_cast(quat);
            quat     = q;
        }

        _M_quaternion   = glm::normalize(_M_quaternion);
        _M_model        = _M_model * glm::mat4_cast(glm::normalize(q));
        _M_euler_angles = glm::eulerAngles(quat);

        update_vectors();

        on_rotate.trigger(this);
    }

    Rotate& Rotate::rotate(const EulerAngle3D& r, const bool& add_values)
    {
        Quaternion q(r);
        update_model(q, add_values);
        return *this;
    }

    Rotate& Rotate::rotate(const EulerAngle1D& angle, const Vector3D& axis, const bool& add_values)
    {
        glm::quat q = glm::rotate(glm::quat(Constants::zero_vector), angle, axis);
        update_model(q, add_values);
        return *this;
    }

    Rotate& Rotate::rotate(const Quaternion& q, const bool& add_values)
    {
        update_model(q, add_values);
        return *this;
    }

    Quaternion Rotate::quaternion() const
    {
        return _M_quaternion;
    }

    const Vector3D& Rotate::front_vector() const
    {
        return _M_front;
    }

    const Vector3D& Rotate::right_vector() const
    {
        return _M_right;
    }

    const Vector3D& Rotate::up_vector() const
    {
        return _M_up;
    }

    // Destructors
    ModelMatrix::~ModelMatrix() = default;
    Translate::~Translate()     = default;
    Rotate::~Rotate()           = default;
    Scale::~Scale()             = default;


}// namespace Engine
