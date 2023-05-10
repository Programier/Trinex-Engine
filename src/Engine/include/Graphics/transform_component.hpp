#pragma once
#include <Core/callback.hpp>
#include <Core/constants.hpp>
#include <Core/engine_types.hpp>
#include <Core/implement.hpp>
#include <Core/object.hpp>
#include <Core/predef.hpp>
#include <type_traits>

namespace Engine
{
    template<bool t, bool r, bool s>
    class TransformComponent;


    template<bool t, bool r, bool s>
    struct TranslateFields {
        CallBacks<void, TransformComponent<t, r, s>*> on_translate;
        CallBacks<void, TransformComponent<t, r, s>*> on_before_translate;
        friend class TransformComponent<t, r, s>;

    private:
        Point3D _M_position = Point3D(0.0f);

        FORCE_INLINE Matrix4f& update_matrix(Matrix4f& matrix) const
        {
            matrix = glm::translate(matrix, _M_position);
            return matrix;
        }
    };

    template<bool A, bool B>
    struct TranslateFields<false, A, B> {
    private:
        FORCE_INLINE Matrix4f& update_matrix(Matrix4f& matrix) const
        {
            return matrix;
        }
    };


    template<bool t, bool r, bool s>
    struct RotateFields {
        CallBacks<void, TransformComponent<t, r, s>*> on_rotate;
        CallBacks<void, TransformComponent<t, r, s>*> on_before_rotate;
        friend class TransformComponent<t, r, s>;

    private:
        Quaternion _M_quaternion = Quaternion(Vector3D(0.f));

        Vector3D _M_front = Constants::OZ;
        Vector3D _M_right = Constants::OX;
        Vector3D _M_up    = Constants::OY;

        FORCE_INLINE Matrix4f& update_matrix(Matrix4f& matrix) const
        {
            matrix = matrix * glm::mat4_cast(_M_quaternion);
            return matrix;
        }
    };

    template<bool A, bool B>
    struct RotateFields<A, false, B> {
        FORCE_INLINE Matrix4f& update_matrix(Matrix4f& matrix) const
        {
            return matrix;
        }
    };


    template<bool t, bool r, bool s>
    struct ScaleFields {
        CallBacks<void, TransformComponent<t, r, s>*> on_scale;
        CallBacks<void, TransformComponent<t, r, s>*> on_before_scale;
        friend class TransformComponent<t, r, s>;

    private:
        Scale3D _M_scale = Point3D(0.0f);
    };

    template<bool A, bool B>
    struct ScaleFields<A, B, false> {
    };


    template<bool translate_enable = true, bool rotate_enable = true, bool scale_enable = true>
    class TransformComponent : public Object
    {

    private:
        mutable Matrix4f _M_matrix  = Matrix4f(1.0f);
        mutable bool _M_need_update = false;

        template<bool t = rotate_enable, typename _ = typename std::enable_if<t>::type>
        TransformComponent& update_model(const Quaternion& q, const bool& add_values);

        FORCE_INLINE void update_matrix() const
        {
            _M_matrix = Constants::identity_matrix;
            _M_translate_fields.update_matrix(_M_rotate_fields.update_matrix(_M_matrix));
            _M_need_update = false;
        }


        ////////////////////// FIELDS /////////////////////
    protected:
        TranslateFields<translate_enable, rotate_enable, scale_enable> _M_translate_fields;
        RotateFields<translate_enable, rotate_enable, scale_enable> _M_rotate_fields;
        ScaleFields<translate_enable, rotate_enable, scale_enable> _M_scale_fields;


        ///////////////////// HEADER IMPLEMENTATION /////////////////////
    public:
        TransformComponent()
        {}
        delete_copy_constructors(TransformComponent);

        const Matrix4f& model() const
        {
            if (_M_need_update)
            {
                update_matrix();
            }
            return _M_matrix;
        }

        FORCE_INLINE TransformComponent& model(const Matrix4f& m)
        {
            return *this;
        }

        ////////////////////// TRANSLATE PART /////////////////////
        template<bool t = translate_enable, typename _ = typename std::enable_if<t>::type>
        TransformComponent& move(const Point1D& x, const Point1D& y, const Point1D& z, bool add_values = true)
        {
            _M_need_update = true;
            _M_translate_fields.on_before_translate.trigger(this);

            if (add_values)
            {
                _M_translate_fields._M_position.x += x;
                _M_translate_fields._M_position.y += y;
                _M_translate_fields._M_position.z += z;
            }
            else
            {
                _M_translate_fields._M_position.x = x;
                _M_translate_fields._M_position.y = y;
                _M_translate_fields._M_position.z = z;
            }

            _M_translate_fields.on_translate.trigger(this);
            return *this;
        }

        template<bool t = translate_enable, typename _ = typename std::enable_if<t>::type>
        FORCE_INLINE TransformComponent& move(const Vector3D& move_vector, bool add_values = true)
        {
            return move(move_vector.x, move_vector.y, move_vector.z, add_values);
        }

        template<bool t = translate_enable, typename _ = typename std::enable_if<t>::type>
        TransformComponent& move(const Point1D& x, const Point1D& y, const Point1D& z, const Vector3D& right,
                                 const Vector3D& up, const Vector3D& front, bool add_values = true)
        {
            Vector3D result_move = (right * x) + (up * y) + (front * z);
            return move(result_move.x, result_move.y, result_move.z, add_values);
        }

        template<bool t = translate_enable, typename _ = typename std::enable_if<t>::type>
        FORCE_INLINE TransformComponent& move(const Vector3D move_vector, const Vector3D& right, const Vector3D& up,
                                              const Vector3D& front, bool add_values = true)
        {
            return move(move_vector.x, move_vector.y, move_vector.z, right, up, front, add_values);
        }

        template<bool t = translate_enable, typename _ = typename std::enable_if<t>::type>
        TransformComponent& move(const Distance& distance, const Vector3D& axis, bool add_value = true)
        {
            Vector3D result_move = distance * glm::normalize(axis);
            return move(result_move.x, result_move.y, result_move.z, add_value);
        }

        template<bool t = translate_enable, typename _ = typename std::enable_if<t>::type>
        FORCE_INLINE const Point3D& position() const
        {
            return _M_translate_fields._M_position;
        }


        ////////////////////// ROTATE PART /////////////////////

        template<bool t = rotate_enable, typename _ = typename std::enable_if<t>::type>
        EulerAngle3D euler_angles() const
        {
            return glm::eulerAngles(_M_rotate_fields._M_quaternion);
        }

        template<bool t = rotate_enable, typename _ = typename std::enable_if<t>::type>
        TransformComponent& rotate(const EulerAngle1D& x, const EulerAngle1D& y, const EulerAngle1D& z,
                                   bool add_values = true)
        {
            return rotate(Quaternion(Vector3D(x, y, z)), add_values);
        }

        template<bool t = rotate_enable, typename _ = typename std::enable_if<t>::type>
        FORCE_INLINE TransformComponent& rotate(const EulerAngle3D& r, bool add_values = true)
        {
            return rotate(Quaternion(r), add_values);
        }

        template<bool t = rotate_enable, typename _ = typename std::enable_if<t>::type>
        TransformComponent& rotate(const EulerAngle1D& angle, const Vector3D& axis, bool add_values = true)
        {
            return rotate(glm::rotate(glm::quat(Constants::zero_vector), angle, axis), add_values);
        }

        template<bool t = rotate_enable, typename _ = typename std::enable_if<t>::type>
        TransformComponent& rotate(const Quaternion& q, bool add_values = true)
        {
            _M_need_update = true;
            _M_rotate_fields.on_before_rotate.trigger(this);

            auto& quat = _M_rotate_fields._M_quaternion;

            if (add_values)
            {
                quat = q * quat;
            }
            else
            {
                quat = q;
            }

            _M_rotate_fields._M_front = glm::normalize(quat * Constants::OZ);
            _M_rotate_fields._M_up    = glm::normalize(quat * Constants::OY);
            _M_rotate_fields._M_right = glm::normalize(quat * Constants::OX);

            _M_rotate_fields.on_rotate.trigger(this);
            return *this;
        }

        template<bool t = rotate_enable, typename _ = typename std::enable_if<t>::type>
        const Quaternion& quaternion() const
        {
            return _M_rotate_fields._M_quaternion;
        }

        template<bool t = rotate_enable, typename _ = typename std::enable_if<t>::type>
        const Vector3D& front_vector() const
        {
            return _M_rotate_fields._M_front;
        }

        template<bool t = rotate_enable, typename _ = typename std::enable_if<t>::type>
        const Vector3D& right_vector() const
        {
            return _M_rotate_fields._M_right;
        }

        template<bool t = rotate_enable, typename _ = typename std::enable_if<t>::type>
        const Vector3D& up_vector() const
        {
            return _M_rotate_fields._M_up;
        }


        ////////////////////// SCALE PART /////////////////////

        template<bool t = scale_enable, typename _ = typename std::enable_if<t>::type>
        const Scale3D& scale() const;

        template<bool t = scale_enable, typename _ = typename std::enable_if<t>::type>
        TransformComponent& scale(const Scale3D& sc, const bool& add_values = true);

        template<bool t = scale_enable, typename _ = typename std::enable_if<t>::type>
        TransformComponent& scale(const Scale1D& x, const Scale1D& y, const Scale1D& z, const bool& add_values = true);
    };


#define COMPONENT_DECLARE template<bool translate_enable, bool rotate_enable, bool scale_enable>
#define COMPONENT_CLASS TransformComponent<translate_enable, rotate_enable, scale_enable>
#define OPTIONAL template<bool t, typename _>

#define OPTIONAL_TYPED_METHOD(type) COMPONENT_DECLARE OPTIONAL type COMPONENT_CLASS
#define OPTIONAL_REFERENCED_METHOD OPTIONAL_TYPED_METHOD(COMPONENT_CLASS&)


    // Translate component


    OPTIONAL_TYPED_METHOD(const Scale3D&)::scale() const
    {
        return _M_scale_fields._M_scale;
    }

    OPTIONAL_REFERENCED_METHOD::scale(const Scale3D& sc, const bool& add_values)
    {
        if (sc.x == 0.f || sc.y == 0.f || sc.z == 0.f)
            return *this;

        _M_scale_fields.on_before_scale.trigger(this);


        if (add_values)
        {
            _M_scale_fields._M_scale *= sc;
        }
        else
        {
            _M_matrix                = glm::scale(_M_matrix, 1.f / _M_scale_fields._M_scale);
            _M_scale_fields._M_scale = sc;
        }

        _M_matrix = glm::scale(_M_matrix, sc);

        _M_scale_fields.on_scale.trigger(this);

        return *this;
    }

    OPTIONAL_REFERENCED_METHOD::scale(const Scale1D& x, const Scale1D& y, const Scale1D& z, const bool& add_values)
    {
        return scale({x, y, z}, add_values);
    }


    namespace TransformComponents
    {
        using MatrixOnly           = TransformComponent<false, false, false>;
        using Translate            = TransformComponent<true, false, false>;
        using TranslateRotate      = TransformComponent<true, true, false>;
        using TranslateScale       = TransformComponent<true, false, true>;
        using TranslateRotateScale = TransformComponent<true, true, true>;
        using Rotate               = TransformComponent<false, true, false>;
        using RotateScale          = TransformComponent<false, true, true>;
        using Scale                = TransformComponent<false, false, true>;
    }// namespace TransformComponents
}// namespace Engine
