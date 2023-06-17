#include <Core/callback.hpp>
#include <Core/constants.hpp>

namespace Engine
{
    class Archive;

    class ENGINE_EXPORT Transform
    {
    private:
        mutable Matrix4f _M_matrix = Matrix4f(1.0f);

        Point3D _M_position      = Vector3D(0.0f);
        Scale3D _M_scale         = Scale3D(1.0f);
        Quaternion _M_quaternion = Quaternion(EulerAngle3D(0.0f));

        mutable byte _M_is_modified : 1 = 0;
        byte _M_revert_front_vector : 1 = 0;

        void update_data() const;


    public:
        const Matrix4f& matrix() const;
        bool is_modified() const;

        Transform& move(const Point1D& x, const Point1D& y, const Point1D& z, bool add_values);
        Transform& move(const Vector3D& move_vector, bool add_values = true);
        Transform& move(const Point1D& x, const Point1D& y, const Point1D& z, const Vector3D& x_axis,
                        const Vector3D& y_axis, const Vector3D& z_axis, bool add_values = true);
        Transform& move(const Vector3D& move_vector, const Vector3D& x_axis, const Vector3D& y_axis,
                        const Vector3D& z_axis, bool add_values = true);
        Transform& move(const Distance& distance, const Vector3D& axis, bool add_value = true);
        const Point3D& position() const;

        const Scale3D& scale() const;
        Transform& scale(const Scale3D& sc, bool mult_values = true);
        Transform& scale(const Scale1D& x, const Scale1D& y, const Scale1D& z, bool mult_values = true);

        EulerAngle3D euler_angles() const;
        Transform& rotate(const EulerAngle1D& x, const EulerAngle1D& y, const EulerAngle1D& z, bool add_values = true);
        Transform& rotate(const EulerAngle3D& r, bool add_values = true);
        Transform& rotate(const EulerAngle1D& angle, const Vector3D& axis, bool add_values = true);
        Transform& rotate(const Quaternion& q, bool add_values = true);
        const Quaternion& quaternion() const;
        Vector3D front_vector() const;
        Vector3D right_vector() const;
        Vector3D up_vector() const;

        String as_string() const;

        friend bool operator & (Archive& ar, Transform& t);

        friend class Camera;
    };

    bool operator & (Archive& ar, Transform& t);

}// namespace Engine
