#pragma once
#include <Core/engine.hpp>
#include <Core/engine_types.hpp>
#include <TemplateFunctional//reference_wrapper.hpp>
#include <glm/gtc/quaternion.hpp>
#include <list>


namespace Engine
{

#define MOVE_METHODS(Obj)                                                                                                   \
    Obj() = default;                                                                                                        \
    Obj(const Obj&) = default;                                                                                              \
    Obj(Obj&&) = default;                                                                                                   \
    Obj& operator=(const Obj&) = default;                                                                                   \
    Obj& operator=(Obj&&) = default;

    CLASS ModelMatrix
    {
    protected:
        ReferenceWrapper<glm::mat4> _M_models[3] = {ReferenceWrapper<glm::mat4>(Constants::identity_matrix),
                                                    ReferenceWrapper<glm::mat4>(Constants::identity_matrix),
                                                    ReferenceWrapper<glm::mat4>(Constants::identity_matrix)};

        ReferenceWrapper<glm::mat4> _M_shift_models[4] = {ReferenceWrapper<glm::mat4>(Constants::identity_matrix),
                                                          ReferenceWrapper<glm::mat4>(Constants::identity_matrix),
                                                          ReferenceWrapper<glm::mat4>(Constants::identity_matrix),
                                                          ReferenceWrapper<glm::mat4>(Constants::identity_matrix)};

        glm::mat4& _M_rotation_matrix();
        glm::mat4& _M_scale_matrix();
        glm::mat4& _M_translate_matrix();

        ModelMatrix& recalculate_shift();


    public:
        const glm::mat4& rotation_matrix() const;
        const glm::mat4& scale_matrix() const;
        const glm::mat4& translate_matrix() const;
        glm::mat4 model() const;
        MOVE_METHODS(ModelMatrix);
    };


    CLASS Translate : public virtual ModelMatrix
    {

    protected:
        ReferenceWrapper<Point3D> _M_position = Point3D(0.f);
        Offset3D _M_shift_position = Constants::zero_vector;

    public:
        Translate& move(const Point1D& x, const Point1D& y, const Point1D& z, const bool& add_values = true);
        Translate& move(const Vector3D& move_vector, const bool& add_values = true);
        Translate& move(const Point1D& x, const Point1D& y, const Point1D& z, const Vector3D& right, const Vector3D& up,
                        const Vector3D& front, const bool& add_values = true);
        Translate& move(const Vector3D move_vector, const Vector3D& right, const Vector3D& up, const Vector3D& front,
                        const bool& add_values = true);
        Translate& move(const Distance& distance, const Vector3D& axis, const bool& add_value = true);
        glm::vec3 position() const;

        auto& shift()
        {
            return _M_shift_position;
        }


        Translate& link_to(Translate & obj);
        MOVE_METHODS(Translate);
        ~Translate();
    };


    CLASS Scale : public virtual ModelMatrix
    {
    protected:
        ReferenceWrapper<Scale3D> _M_scale = Scale3D(1.f);

    public:
        const Scale3D& scale() const;
        Scale& scale(const Scale3D& sc, const bool& add_values = true);
        Scale& scale(const Scale1D& x, const Scale1D& y, const Scale1D& z, const bool& add_values = true);
        Scale& link_to(Scale & obj);
        MOVE_METHODS(Scale);
    };


    CLASS Rotate : public virtual ModelMatrix
    {
    private:
        void update_model(const Quaternion& q, const bool& add_values);

    protected:
        ReferenceWrapper<Quaternion> _M_quaternion = Quaternion(Vector3D(0.f));
        ReferenceWrapper<EulerAngle3D> _M_euler_angles = EulerAngle3D(0.f);
        ReferenceWrapper<Vector3D> _M_front = Constants::OZ;
        ReferenceWrapper<Vector3D> _M_right = Constants::OX;
        ReferenceWrapper<Vector3D> _M_up = Constants::OY;

    public:
        const EulerAngle3D& euler_angles() const;
        Rotate& rotate(const EulerAngle1D& x, const EulerAngle1D& y, const EulerAngle1D& z, const bool& add_values = true);
        Rotate& rotate(const EulerAngle3D& r, const bool& add_values = true);
        Rotate& rotate(const EulerAngle1D& angle, const Vector3D& axis, const bool& add_values = true);
        Rotate& rotate(const Quaternion& q, const bool& add_values = true);
        const Quaternion& quaternion() const;
        const Vector3D& front_vector() const;
        const Vector3D& right_vector() const;
        const Vector3D& up_vector() const;

        Rotate& link_to(Rotate & obj);
        MOVE_METHODS(Rotate);
    };


    template<typename... BaseClasses>
    class BasicObject : public BaseClasses...
    {
    };

#undef MOVE_METHODS

}// namespace Engine
