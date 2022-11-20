#pragma once
#include <Core/engine.hpp>
#include <Core/engine_types.hpp>
#include <TemplateFunctional/reference_wrapper.hpp>
#include <glm/gtc/quaternion.hpp>
#include <list>
#include <unordered_set>
#include <Core/instance.hpp>

namespace Engine
{

    CLASS ModelMatrix : public virtual ObjectInstance
    {
    protected:
        ReferenceWrapper<glm::mat4> _M_model = ReferenceWrapper<glm::mat4>(Constants::identity_matrix);
        ReferenceWrapper<glm::mat4> _M_shift = ReferenceWrapper<glm::mat4>(Constants::identity_matrix);

        ReferenceWrapper<Point3D> _M_position = ReferenceWrapper<Point3D>(Constants::zero_vector);
        ReferenceWrapper<Point3D> _M_scale = ReferenceWrapper<Point3D>(Constants::identity_vector);
        ReferenceWrapper<Point3D> _M_rotation = ReferenceWrapper<Point3D>(Constants::zero_vector);

        Point3D _M_shift_position = Constants::zero_vector;
        Point3D _M_shift_scale = Constants::identity_vector;
        Point3D _M_shift_rotation = Constants::zero_vector;

        ReferenceWrapper<Quaternion> _M_quaternion = ReferenceWrapper<Quaternion>(Quaternion(Vector3D(0.f)));

        ReferenceWrapper<EulerAngle3D> _M_euler_angles = ReferenceWrapper<EulerAngle3D>(EulerAngle3D(0.f));
        mutable Vector3D _M_front = Constants::OZ;
        mutable Vector3D _M_right = Constants::OX;
        mutable Vector3D _M_up = Constants::OY;

        ModelMatrix& update_data();


        ModelMatrix& update_shift_data();
        const ModelMatrix& update_vectors() const;

    protected:
        std::unordered_set<void(*)(ModelMatrix*)> _M_on_before_set_model;
        std::unordered_set<void(*)(ModelMatrix*)> _M_on_set_model;

    public:
        ModelMatrix();
        ModelMatrix(const ModelMatrix&);
        ModelMatrix(ModelMatrix &&);
        ModelMatrix& operator=(const ModelMatrix&);
        ModelMatrix& operator=(ModelMatrix&&);
        glm::mat4 model() const;
        ModelMatrix& model(const glm::mat4& m);
        ModelMatrix& link_to(ModelMatrix & obj);

        virtual ~ModelMatrix();
    };


    CLASS Translate : public virtual ModelMatrix
    {

    protected:
        std::unordered_set<void (*)(Translate*)> _M_on_translate;
        std::unordered_set<void (*)(Translate*)> _M_on_before_translate;

    public:
        Translate();
        Translate(const Translate&);
        Translate(Translate &&);
        Translate& operator=(const Translate&);
        Translate& operator=(Translate&&);

        Translate& move(const Point1D& x, const Point1D& y, const Point1D& z, const bool& add_values = true);
        Translate& move(const Vector3D& move_vector, const bool& add_values = true);
        Translate& move(const Point1D& x, const Point1D& y, const Point1D& z, const Vector3D& right, const Vector3D& up,
                        const Vector3D& front, const bool& add_values = true);
        Translate& move(const Vector3D move_vector, const Vector3D& right, const Vector3D& up, const Vector3D& front,
                        const bool& add_values = true);
        Translate& move(const Distance& distance, const Vector3D& axis, const bool& add_value = true);
        Point3D position() const;

        const Point3D& shift() const;

        virtual ~Translate();
    };


    CLASS Scale : public virtual ModelMatrix
    {
    protected:
        std::unordered_set<void (*)(Scale*)> _M_on_scale;
        std::unordered_set<void (*)(Scale*)> _M_on_before_scale;

    public:
        Scale();
        Scale(const Scale&);
        Scale(Scale &&);
        Scale& operator=(const Scale&);
        Scale& operator=(Scale&&);
        Scale3D scale() const;
        Scale& scale(const Scale3D& sc, const bool& add_values = true);
        Scale& scale(const Scale1D& x, const Scale1D& y, const Scale1D& z, const bool& add_values = true);

        virtual ~Scale();
    };


    CLASS Rotate : public virtual ModelMatrix
    {
    private:
        void update_model(const Quaternion& q, const bool& add_values);

    protected:
        std::unordered_set<void (*)(Rotate*)> _M_on_rotate;
        std::unordered_set<void (*)(Rotate*)> _M_on_before_rotate;

    public:
        Rotate();
        Rotate(const Rotate&);
        Rotate(Rotate &&);
        Rotate& operator=(const Rotate&);
        Rotate& operator=(Rotate&&);

        EulerAngle3D euler_angles() const;
        Rotate& rotate(const EulerAngle1D& x, const EulerAngle1D& y, const EulerAngle1D& z, const bool& add_values = true);
        Rotate& rotate(const EulerAngle3D& r, const bool& add_values = true);
        Rotate& rotate(const EulerAngle1D& angle, const Vector3D& axis, const bool& add_values = true);
        Rotate& rotate(const Quaternion& q, const bool& add_values = true);
        Quaternion quaternion() const;

        virtual Vector3D front_vector(bool update = true) const;
        const Vector3D& right_vector(bool update = true) const;
        const Vector3D& up_vector(bool update = true) const;

        virtual ~Rotate();
    };


    template<typename... BaseClasses>
    class BasicObject : public BaseClasses...
    {
    public:
        virtual ~BasicObject() = default;
    };

#undef MOVE_METHODS

}// namespace Engine
