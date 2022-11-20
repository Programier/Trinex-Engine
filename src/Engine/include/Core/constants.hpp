#pragma once
#include <Core/engine_types.hpp>
#include <Core/export.hpp>

namespace Engine
{
    STRUCT Constants
    {
        static const unsigned int processor_count;
        static const Vector3D OX;
        static const Vector3D OY;
        static const Vector3D OZ;
        static const float PI;
        static const float E;
        static const glm::mat4 identity_matrix;
        static const glm::mat4 zero_matrix;
        static const Vector4D identity_vector;
        static const Vector4D zero_vector;
        static const std::string ShaderModeValue;
        static const float min_positive_float;
        static const Vector3D min_positive_vector;
    };
}
