#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Core/engine_types.hpp>
#include <Core/logger.hpp>
#include <Core/shader_types.hpp>
#include <Graphics/shader.hpp>
#include <api.hpp>
#include <fstream>
#include <glm/gtc/type_ptr.hpp>
#include <sstream>


namespace Engine
{

    static const Map<std::type_index, typeof(ShaderDataType::Bool)> types = {
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

    ENGINE_EXPORT void ShaderDataType::private_type_of(ShaderDataType& result, const std::type_index& index)
    {
        result.type = types.find(index)->second;
    }

    register_class(Engine::Shader, Engine::ApiObject);

    Shader::Shader()
    {}

    static Identifier _M_current = 0;

    Shader::Shader(const PipelineCreateInfo& params)
    {
        load(params);
    }


    Shader& Shader::load(const PipelineCreateInfo& params)
    {
        destroy();
        EngineInstance::instance()->api_interface()->create_shader(_M_ID, params);
        return *this;
    }


    const Shader& Shader::use() const
    {
        //  if (_M_ID != _M_current)
        {
            _M_current = _M_ID;
            EngineInstance::instance()->api_interface()->use_shader(_M_ID);
        }

        return *this;
    }

    void Shader::unbind()
    {
        EngineInstance::instance()->api_interface()->use_shader(0);
    }


}// namespace Engine
