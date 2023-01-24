#include <assimp/matrix3x3.h>
#include <assimp/matrix4x4.h>
#include <assimp/quaternion.h>
#include <assimp/vector2.h>
#include <assimp/vector3.h>

#include <Core/assimp_helpers.hpp>


namespace Engine::AssimpHelpers
{

    template<typename Type>
    inline const Type& parse(AssimpObject object)
    {
        return *static_cast<const Type*>(object);
    }


    ENGINE_EXPORT Matrix3f get_matrix3(AssimpObject matrix)
    {
        Matrix3f to;
        const auto& from = parse<aiMatrix3x3>(matrix);
        to[0][0] = from.a1;
        to[1][0] = from.a2;
        to[2][0] = from.a3;
        to[0][1] = from.b1;
        to[1][1] = from.b2;
        to[2][1] = from.b3;
        to[0][2] = from.c1;
        to[1][2] = from.c2;
        to[2][2] = from.c3;
        return to;
    }

    ENGINE_EXPORT Matrix4f get_matrix4(AssimpObject matrix)
    {

        Matrix4f to;
        const auto& from = parse<aiMatrix4x4>(matrix);
        to[0][0] = from.a1;
        to[1][0] = from.a2;
        to[2][0] = from.a3;
        to[3][0] = from.a4;
        to[0][1] = from.b1;
        to[1][1] = from.b2;
        to[2][1] = from.b3;
        to[3][1] = from.b4;
        to[0][2] = from.c1;
        to[1][2] = from.c2;
        to[2][2] = from.c3;
        to[3][2] = from.c4;
        to[0][3] = from.d1;
        to[1][3] = from.d2;
        to[2][3] = from.d3;
        to[3][3] = from.d4;
        return to;
    }

    ENGINE_EXPORT Vector2D get_vector2(AssimpObject vector)
    {
        const auto& from = parse<aiVector2D>(vector);
        return Vector2D(from.x, from.y);
    }

    ENGINE_EXPORT Vector3D get_vector3(AssimpObject vector)
    {
        const auto& from = parse<aiVector3D>(vector);
        return Vector3D(from.x, from.y, from.z);
    }

    ENGINE_EXPORT Vector2D get_vector4(AssimpObject vector)
    {
        const auto& from = parse<aiVector2D>(vector);
        return Vector2D(from.x, from.y);
    }

    ENGINE_EXPORT Quaternion get_quaternion(AssimpObject quaternion)
    {
        Quaternion result;
        result.x = parse<aiQuaternion>(quaternion).x;
        result.y = parse<aiQuaternion>(quaternion).y;
        result.z = parse<aiQuaternion>(quaternion).z;
        result.w = parse<aiQuaternion>(quaternion).w;
        return result;
    }
}// namespace Engine::AssimpHelpers
