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
        static const Matrix4f identity_matrix;
        static const Matrix4f zero_matrix;
        static const Vector4D identity_vector;
        static const Vector4D zero_vector;
        static const std::string ShaderModeValue;
        static const float min_positive_float;
        static const Vector3D min_positive_vector;
        static const ArrayIndex index_none;
        static const IntVector4D int_zero_vector;
        static const IntVector4D int_identity_vector;
        static const IntVector4D uint_zero_vector;
        static const IntVector4D uint_identity_vector;
    };
}
