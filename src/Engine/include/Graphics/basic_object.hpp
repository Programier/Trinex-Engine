#pragma once
#include <Core/callback.hpp>
#include <Core/engine.hpp>
#include <Core/engine_types.hpp>
#include <Core/object.hpp>
#include <glm/gtc/quaternion.hpp>



namespace Engine
{

    class ENGINE_EXPORT ModelMatrix
    {
    protected:
        Matrix4f _M_model   = Constants::identity_matrix;
        Point3D _M_position = Constants::zero_vector;
        Point3D _M_scale    = Constants::identity_vector;
        Point3D _M_rotation = Constants::zero_vector;

        Quaternion _M_quaternion     = Quaternion(Vector3D(0.f));
        EulerAngle3D _M_euler_angles = Constants::zero_vector;

        Vector3D _M_front = Constants::OZ;
        Vector3D _M_right = Constants::OX;
        Vector3D _M_up    = Constants::OY;


        ModelMatrix& update_data();
        ModelMatrix& update_vectors();

    public:
        CallBacks<void, ModelMatrix*> on_before_set_model;
        CallBacks<void, ModelMatrix*> on_set_model;

        constructor_hpp(ModelMatrix);
        delete_copy_constructors(ModelMatrix);

        const Matrix4f& model() const;
        ModelMatrix& model(const glm::mat4& m);
        virtual ~ModelMatrix();
    };


    class ENGINE_EXPORT Translate : public virtual ModelMatrix
    {
    public:
        CallBacks<void, Translate*> on_translate;
        CallBacks<void, Translate*> on_before_translate;

        constructor_hpp(Translate);
        delete_copy_constructors(Translate);

        Translate& move(const Point1D& x, const Point1D& y, const Point1D& z, const bool& add_values = true);
        Translate& move(const Vector3D& move_vector, const bool& add_values = true);
        Translate& move(const Point1D& x, const Point1D& y, const Point1D& z, const Vector3D& right, const Vector3D& up,
                        const Vector3D& front, const bool& add_values = true);
        Translate& move(const Vector3D move_vector, const Vector3D& right, const Vector3D& up, const Vector3D& front,
                        const bool& add_values = true);
        Translate& move(const Distance& distance, const Vector3D& axis, const bool& add_value = true);
        const Point3D& position() const;
        virtual ~Translate();
    };


    class ENGINE_EXPORT Scale : public virtual ModelMatrix
    {

    public:
        CallBacks<void, Scale*> on_scale;
        CallBacks<void, Scale*> on_before_scale;

        constructor_hpp(Scale);
        delete_copy_constructors(Scale);
        const Scale3D& scale() const;
        Scale& scale(const Scale3D& sc, const bool& add_values = true);
        Scale& scale(const Scale1D& x, const Scale1D& y, const Scale1D& z, const bool& add_values = true);

        virtual ~Scale();
    };


    class ENGINE_EXPORT Rotate : public virtual ModelMatrix
    {
    private:
        void update_model(const Quaternion& q, const bool& add_values);

    public:
        CallBacks<void, Rotate*> on_rotate;
        CallBacks<void, Rotate*> on_before_rotate;

        constructor_hpp(Rotate);
        delete_copy_constructors(Rotate);

        const EulerAngle3D& euler_angles() const;
        Rotate& rotate(const EulerAngle1D& x, const EulerAngle1D& y, const EulerAngle1D& z,
                       const bool& add_values = true);
        Rotate& rotate(const EulerAngle3D& r, const bool& add_values = true);
        Rotate& rotate(const EulerAngle1D& angle, const Vector3D& axis, const bool& add_values = true);
        Rotate& rotate(const Quaternion& q, const bool& add_values = true);
        Quaternion quaternion() const;

        const Vector3D& front_vector() const;
        const Vector3D& right_vector() const;
        const Vector3D& up_vector() const;
        virtual ~Rotate();
    };


    template<typename... BaseClasses>
    class BasicObject : public BaseClasses..., public virtual ModelMatrix, public Object
    {


    public:
        delete_copy_constructors(BasicObject);
        BasicObject()
        {}
        virtual ~BasicObject() = default;
    };

#undef MOVE_METHODS

}// namespace Engine
