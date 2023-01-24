#pragma once
#include <Core/engine_types.hpp>
#include <Core/export.hpp>


namespace Engine::AssimpHelpers
{
    ENGINE_EXPORT Matrix3f get_matrix3(AssimpObject matrix);
    ENGINE_EXPORT Matrix4f get_matrix4(AssimpObject matrix);
    ENGINE_EXPORT Vector3D get_vector3(AssimpObject vector);
    ENGINE_EXPORT Vector2D get_vector2(AssimpObject vector);
    ENGINE_EXPORT Quaternion get_quaternion(AssimpObject quaternion);
}
