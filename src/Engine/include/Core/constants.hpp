#pragma once
#include <Core/engine_types.hpp>

namespace Engine
{
    struct ENGINE_EXPORT Constants {
        static const uint_t processor_count;
        static const Vector3D OX;
        static const Vector3D OY;
        static const Vector3D OZ;
        static const float PI;
        static const float E;
        static const Matrix4f identity_matrix;
        static const Matrix4f zero_matrix;
        static const Vector4D identity_vector;
        static const Vector4D zero_vector;
        static const float min_positive_float;
        static const Vector3D min_positive_vector;
        static const ArrayIndex index_none;
        static const HashIndex invalid_hash;
        static const size_t max_size;
        static const IntVector4D int_zero_vector;
        static const IntVector4D int_identity_vector;
        static const IntVector4D uint_zero_vector;
        static const IntVector4D uint_identity_vector;
        static const String package_extention;
        static const String name_separator;
        static const PriorityIndex max_priority;
        static const String default_entry_point;
        static const String library_load_function_name;
        static const String script_extension;
        static const String script_byte_code_extension;
    };
}// namespace Engine
