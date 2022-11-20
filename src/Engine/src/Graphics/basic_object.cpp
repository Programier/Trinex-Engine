#include <Core/engine.hpp>
#include <Graphics/basic_object.hpp>
#include <algorithm>
#include <glm/ext.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <numeric>

#define TRANSLATE_INDEX 0
#define ROTATE_INDEX 0
#define SCALE_INDEX 0


#define get_model(index) _M_models[index].is_null() ? Constants::identity_matrix : _M_models[index].get();

#define NEW_VARIANT 1

namespace Engine
{


    ModelMatrix::ModelMatrix() = default;
    ModelMatrix::ModelMatrix(const ModelMatrix& m)
    {
        *this = m;
    }

    ModelMatrix::ModelMatrix(ModelMatrix&& m)
    {
        *this = std::move(m);
    }


    ModelMatrix& ModelMatrix::operator=(const ModelMatrix& m)
    {
        if (this == &m)
            return *this;

        _M_model = ReferenceWrapper<glm::mat4>(m._M_model);
        _M_shift = ReferenceWrapper<glm::mat4>(m._M_shift);


        _M_position = ReferenceWrapper<Vector3D>(m._M_position);
        _M_scale = ReferenceWrapper<Vector3D>(m._M_scale);
        _M_rotation = ReferenceWrapper<Vector3D>(m._M_rotation);

        _M_shift_position = m._M_shift_position;
        _M_shift_scale = m._M_shift_scale;
        _M_shift_rotation = m._M_shift_rotation;

        _M_quaternion = ReferenceWrapper<Quaternion>(m._M_quaternion);
        _M_euler_angles = ReferenceWrapper<EulerAngle3D>(m._M_euler_angles);
        _M_front = m._M_up;
        _M_right = m._M_front;
        _M_up = m._M_right;

        return *this;
    }

    ModelMatrix& ModelMatrix::operator=(ModelMatrix&& m)
    {
        _M_model = std::move(m._M_model);
        _M_shift = std::move(m._M_shift);

        _M_position = std::move(m._M_position);
        _M_scale = std::move(m._M_scale);
        _M_rotation = std::move(m._M_rotation);

        _M_shift_position = std::move(m._M_shift_position);
        _M_shift_scale = std::move(m._M_shift_scale);
        _M_shift_rotation = std::move(m._M_shift_rotation);

        _M_quaternion = std::move(m._M_quaternion);
        _M_euler_angles = std::move(m._M_euler_angles);
        _M_front = std::move(m._M_up);
        _M_right = std::move(m._M_front);
        _M_up = std::move(m._M_right);
        return *this;
    }

    glm::mat4 ModelMatrix::model() const
    {
        return _M_shift.get() * _M_model.get();
    }

    ModelMatrix& ModelMatrix::link_to(ModelMatrix& obj)
    {
        _M_model = obj._M_model;
        return *this;
    }

    ModelMatrix& ModelMatrix::update_shift_data()
    {
        glm::vec3 skew;
        glm::vec4 perspective;
        Quaternion q;

        glm::decompose(_M_shift.get(), _M_shift_scale, q, _M_shift_position, skew, perspective);
        _M_shift_rotation = glm::eulerAngles(q);
        return *this;
    }

    ModelMatrix& ModelMatrix::update_data()
    {
        glm::vec3 skew;
        glm::vec4 perspective;

        glm::decompose(_M_model.get(), _M_scale.get(), _M_quaternion.get(), _M_position.get(), skew, perspective);

        _M_euler_angles = glm::eulerAngles(_M_quaternion.get());
        update_vectors();
        return *this;
    }

    const ModelMatrix& ModelMatrix::update_vectors() const
    {
        Quaternion tmp(_M_euler_angles.get() + _M_shift_rotation);
        _M_front = glm::normalize(tmp * Constants::OZ);
        _M_up = glm::normalize(tmp * Constants::OY);
        _M_right = glm::normalize(tmp * Constants::OX);
        return *this;
    }

    ModelMatrix& ModelMatrix::model(const glm::mat4& m)
    {
        for (auto func : _M_on_before_set_model) func(this);

        _M_model.get() = m;
        update_data();

        for (auto func : _M_on_set_model) func(this);
        return *this;
    }


    //      TRANSLATE

    Translate::Translate() = default;
    Translate::Translate(const Translate& obj)
    {
        *this = obj;
    }

    Translate::Translate(Translate&& obj)
    {
        *this = std::move(obj);
    }

    Translate& Translate::operator=(const Translate& obj)
    {
        return *this;
    }

    Translate& Translate::operator=(Translate&& obj)
    {
        return *this;
    }

    Point3D Translate::position() const
    {
        return _M_position.get() + _M_shift_position;
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

    Translate& Translate::move(const Vector3D move_vector, const Vector3D& right, const Vector3D& up, const Vector3D& front,
                               const bool& add_values)
    {
        for (auto func : _M_on_before_translate)
        {
            func(this);
        }


        glm::vec3 result_move = (right * move_vector.x) + (up * move_vector.y) + (front * move_vector.z);

        if (add_values)
        {
            this->_M_position.get() += result_move;
            _M_model.get() = glm::translate(_M_model.get(), result_move);
        }
        else
        {
            _M_model.get() = glm::translate(_M_model.get(), -_M_position.get() + result_move);
            _M_position = move_vector;
        }


        for (auto func : _M_on_translate)
        {
            func(this);
        }

        return *this;
    }

    Translate& Translate::move(const Distance& distance, const glm::vec3& axis, const bool& add_value)
    {
        return move(axis * distance, Constants::OX, Constants::OY, Constants::OZ, add_value);
    }


    //          SCALE

    Scale::Scale() = default;
    Scale::Scale(const Scale& obj)
    {
        *this = obj;
    }

    Scale::Scale(Scale&& obj)
    {
        *this = std::move(obj);
    }

    Scale& Scale::operator=(const Scale& obj)
    {
        if (this != &obj)
        {
            this->_M_scale = obj._M_scale;
        }
        return *this;
    }

    Scale& Scale::operator=(Scale&& obj)
    {
        if (this != &obj)
        {
            this->_M_scale = std::move(obj._M_scale);
        }
        return *this;
    }

    Scale3D Scale::scale() const
    {
        return _M_scale.get() * _M_shift_scale;
    }

    Scale& Scale::scale(const Scale3D& sc, const bool& add_values)
    {
        if (sc.x == 0.f || sc.y == 0.f || sc.z == 0.f)
            return *this;

        for (auto func : _M_on_before_scale)
        {
            func(this);
        }


        if (add_values)
        {
            _M_scale.get() *= sc;
        }
        else
        {
            _M_model.get() = glm::scale(_M_model.get(), 1.f / _M_scale.get());
            _M_scale = sc;
        }

        _M_model.get() = glm::scale(_M_model.get(), sc);

        for (auto func : _M_on_scale)
        {
            func(this);
        }

        return *this;
    }

    Scale& Scale::scale(const Scale1D& x, const Scale1D& y, const Scale1D& z, const bool& add_values)
    {
        return scale({x, y, z}, add_values);
    }


    //      ROTATION

    Rotate::Rotate() = default;
    Rotate::Rotate(const Rotate& obj)
    {
        *this = obj;
    }

    Rotate::Rotate(Rotate&& obj)
    {
        *this = std::move(obj);
    }

    Rotate& Rotate::operator=(const Rotate& obj)
    {
        if (this != &obj)
        {
            this->_M_quaternion = obj._M_quaternion;
            this->_M_euler_angles = obj._M_euler_angles;
            this->_M_front = obj._M_front;
            this->_M_right = obj._M_right;
            this->_M_up = obj._M_up;
        }

        return *this;
    }

    Rotate& Rotate::operator=(Rotate&& obj)
    {
        if (this != &obj)
        {
            this->_M_quaternion = std::move(obj._M_quaternion);
            this->_M_euler_angles = std::move(obj._M_euler_angles);
            this->_M_front = std::move(obj._M_front);
            this->_M_right = std::move(obj._M_right);
            this->_M_up = std::move(obj._M_up);
        }

        return *this;
    }

    EulerAngle3D Rotate::euler_angles() const
    {
        return _M_euler_angles.get() + _M_shift_rotation;
    }

    Rotate& Rotate::rotate(const EulerAngle1D& x, const EulerAngle1D& y, const EulerAngle1D& z, const bool& add_values)
    {
        return rotate(glm::quat(glm::vec3(x, y, z)), add_values);
    }

    void Rotate::update_model(const Quaternion& q, const bool& add_values)
    {
        for (auto func : _M_on_before_rotate)
        {
            func(this);
        }

        auto& quat = _M_quaternion.get();

        if (add_values)
            _M_quaternion = q * quat;
        else
        {
            quat.w = -quat.w;
            _M_model.get() = _M_model.get() * glm::mat4_cast(quat);
            quat = q;
        }

        _M_quaternion.get() = glm::normalize(_M_quaternion.get());
        _M_model.get() = _M_model.get() * glm::mat4_cast(glm::normalize(q));
        _M_euler_angles.get() = glm::eulerAngles(quat);

        update_vectors();

        for (auto func : _M_on_rotate)
        {
            func(this);
        }
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

    Quaternion Rotate::quaternion() const
    {
        return Quaternion(_M_shift_scale) * _M_quaternion.get();
    }

    Vector3D Rotate::front_vector(bool update) const
    {
        if (update)
            update_vectors();
        return _M_front;
    }

    const Vector3D& Rotate::right_vector(bool update) const
    {
        if (update)
            update_vectors();
        return _M_right;
    }

    const Vector3D& Rotate::up_vector(bool update) const
    {
        if (update)
            update_vectors();
        return _M_up;
    }

    // Destructors
    ModelMatrix::~ModelMatrix() = default;
    Translate::~Translate() = default;
    Rotate::~Rotate() = default;
    Scale::~Scale() = default;


}// namespace Engine
