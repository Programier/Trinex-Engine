#include <Core/buffer_manager.hpp>
#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Core/engine_config.hpp>
#include <Core/engine_types.hpp>
#include <Core/logger.hpp>
#include <Graphics/g_buffer.hpp>
#include <Graphics/mesh_component.hpp>
#include <Graphics/rhi.hpp>
#include <Graphics/shader.hpp>
#include <fstream>
#include <glm/gtc/type_ptr.hpp>
#include <sstream>


namespace Engine
{
    using LocalShaderTypes = Map<std::type_index, typeof(ShaderDataType::Bool)>;

    static const LocalShaderTypes& shader_types()
    {
        static const LocalShaderTypes types = {
                {std::type_index(typeid(bool)), ShaderDataType::Bool},
                {std::type_index(typeid(int_t)), ShaderDataType::Int},
                {std::type_index(typeid(uint_t)), ShaderDataType::UInt},
                {std::type_index(typeid(float)), ShaderDataType::Float},
                {std::type_index(typeid(Vector2D)), ShaderDataType::Vec2},
                {std::type_index(typeid(Vector3D)), ShaderDataType::Vec3},
                {std::type_index(typeid(Vector4D)), ShaderDataType::Vec4},
                {std::type_index(typeid(IntVector2D)), ShaderDataType::IVec2},
                {std::type_index(typeid(IntVector3D)), ShaderDataType::IVec3},
                {std::type_index(typeid(IntVector4D)), ShaderDataType::IVec4},
                {std::type_index(typeid(UIntVector2D)), ShaderDataType::UVec2},
                {std::type_index(typeid(UIntVector3D)), ShaderDataType::UVec3},
                {std::type_index(typeid(UIntVector4D)), ShaderDataType::UVec4},
                {std::type_index(typeid(BoolVector2D)), ShaderDataType::BVec2},
                {std::type_index(typeid(BoolVector3D)), ShaderDataType::BVec3},
                {std::type_index(typeid(BoolVector4D)), ShaderDataType::BVec4},
                {std::type_index(typeid(Matrix2f)), ShaderDataType::Mat2},
                {std::type_index(typeid(Matrix3f)), ShaderDataType::Mat3},
                {std::type_index(typeid(Matrix4f)), ShaderDataType::Mat4},
        };

        return types;
    }

    ENGINE_EXPORT void ShaderDataType::private_type_of(ShaderDataType& result, const std::type_index& index)
    {
        result.type = shader_types().find(index)->second;
    }


    implement_class(Shader, "Engine");
    implement_default_initialize_class(Shader);

    implement_class(VertexShader, "Engine");
    implement_default_initialize_class(VertexShader);

    implement_class(FragmentShader, "Engine");
    implement_default_initialize_class(FragmentShader);


    Shader& Shader::rhi_create()
    {
        Super::rhi_create();
        return *this;
    }

    VertexShader& VertexShader::rhi_create()
    {
        Super::rhi_create();
        _M_rhi_shader = engine_instance->rhi()->create_vertex_shader(this);
        return *this;
    }

    FragmentShader& FragmentShader::rhi_create()
    {
        Super::rhi_create();
        _M_rhi_shader = engine_instance->rhi()->create_fragment_shader(this);
        return *this;
    }
}// namespace Engine
