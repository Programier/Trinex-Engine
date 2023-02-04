#include <Core/engine.hpp>
#include <Core/engine_types.hpp>
#include <Core/logger.hpp>
#include <Graphics/shader.hpp>
#include <api.hpp>
#include <fstream>
#include <glm/gtc/type_ptr.hpp>
#include <sstream>
#include <vector>


static void read_file(const std::string& filename, std::vector<Engine::byte>& out)
{
    out.clear();
    if (filename.empty())
    {
        return;
    }
    std::ifstream input_file(filename, std::ios_base::binary);
    if (!input_file.is_open())
    {
        Engine::logger->log("Shader: Failed to open %s\n", filename.c_str());
        return;
    }

    input_file.seekg(0, std::ios_base::end);
    std::size_t bytes = static_cast<std::size_t>(input_file.tellg());
    input_file.seekg(0, std::ios_base::beg);

    out.resize(bytes);

    input_file.read(reinterpret_cast<char*>(out.data()), bytes);
}


namespace Engine
{
    declare_instance_info_cpp(Shader);

    constructor_cpp(Shader)
    {}

    static ObjID _M_current = 0;

    Shader::Shader(const ShaderParams& params)
    {
        load(params);
    }


    Shader& Shader::load(const ShaderParams& params)
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


    const Shader& Shader::set(const std::string& value_name, float value) const
    {
        EngineInstance::instance()->api_interface()->shader_value(_M_ID, value_name, value);
        return *this;
    }

    const Shader& Shader::set(const std::string& value_name, int value) const
    {
        EngineInstance::instance()->api_interface()->shader_value(_M_ID, value_name, value);
        return *this;
    }

    const Shader& Shader::set(const std::string& value_name, const glm::mat4& value) const
    {
        EngineInstance::instance()->api_interface()->shader_value(_M_ID, value_name, value);
        return *this;
    }

    const Shader& Shader::set(const std::string& value_name, const glm::mat3& value) const
    {
        EngineInstance::instance()->api_interface()->shader_value(_M_ID, value_name, value);
        return *this;
    }

    const Shader& Shader::set(const std::string& value_name, const bool& value) const
    {
        EngineInstance::instance()->api_interface()->shader_value(_M_ID, value_name, value);
        return *this;
    }

    const Shader& Shader::set(const std::string& value_name, const glm::vec2& value) const
    {
        EngineInstance::instance()->api_interface()->shader_value(_M_ID, value_name, value);
        return *this;
    }

    const Shader& Shader::set(const std::string& value_name, const glm::vec3& value) const
    {
        EngineInstance::instance()->api_interface()->shader_value(_M_ID, value_name, value);
        return *this;
    }

    const Shader& Shader::set(const std::string& value_name, const Vector4D& value) const
    {
        EngineInstance::instance()->api_interface()->shader_value(_M_ID, value_name, value);
        return *this;
    }

    const Shader& Shader::set(const String& value_name, void* data) const
    {
        EngineInstance::instance()->api_interface()->shader_value(_M_ID, value_name, data);
        return *this;
    }


}// namespace Engine
