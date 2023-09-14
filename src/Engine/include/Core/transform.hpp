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

        Transform& move(Point1D x, Point1D y, Point1D z, bool add_values = true);
        Transform& move(const Vector3D& move_vector, bool add_values = true);
        Transform& move(Point1D x, Point1D y, Point1D z, const Vector3D& x_axis, const Vector3D& y_axis,
                        const Vector3D& z_axis, bool add_values = true);
        Transform& move(const Vector3D& move_vector, const Vector3D& x_axis, const Vector3D& y_axis,
                        const Vector3D& z_axis, bool add_values = true);
        Transform& move(Distance distance, const Vector3D& axis, bool add_value = true);
        const Point3D& position() const;

        const Scale3D& scale() const;
        Transform& scale(const Scale3D& sc, bool mult_values = true);
        Transform& scale(float x, float y, float z, bool mult_values = true);

        EulerAngle3D euler_angles() const;
        Transform& rotate(float x, float y, float z, bool add_values = true);
        Transform& rotate(const EulerAngle3D& r, bool add_values = true);
        Transform& rotate(float angle, const Vector3D& axis, bool add_values = true);
        Transform& rotate(const Quaternion& q, bool add_values = true);
        const Quaternion& quaternion() const;
        Vector3D front_vector() const;
        Vector3D right_vector() const;
        Vector3D up_vector() const;

        String as_string() const;

        friend bool operator&(Archive& ar, Transform& t);

        friend class Camera;
    };

    bool operator&(Archive& ar, Transform& t);

}// namespace Engine
