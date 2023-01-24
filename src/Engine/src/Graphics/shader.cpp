#include <Core/engine_types.hpp>
#include <Core/logger.hpp>
#include <Graphics/shader.hpp>
#include <api_funcs.hpp>
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

    Shader::Shader(const std::string& name, const std::string& vertex, const std::string& fragment,
                   const std::string& compute, const std::string& geometry, ShaderSourceType type)
    {
        load(name, vertex, fragment, compute, geometry, type);
    }

    Shader& Shader::load(const ShaderParams& params)
    {
        destroy();
        create_shader(_M_ID, params);
        return *this;
    }

    Shader& Shader::load(const std::string& name, const std::string& vertex, const std::string& fragment,
                         const std::string& compute, const std::string& geometry, ShaderSourceType type)
    {
        destroy();

        ShaderParams params;
        params.name = name;
        params.source_type = type;

        read_file(vertex, params.vertex);
        read_file(fragment, params.fragment);
        read_file(compute, params.compute);
        read_file(geometry, params.geometry);

        create_shader(_M_ID, params);

        return *this;
    }


    const Shader& Shader::use() const
    {
        if (_M_ID != _M_current)
        {
            _M_current = _M_ID;
            use_shader(_M_ID);
        }

        return *this;
    }


    const Shader& Shader::set(const std::string& value_name, float value) const
    {
        set_shader_float_value(_M_ID, value_name, value);
        return *this;
    }

    const Shader& Shader::set(const std::string& value_name, int value) const
    {
        set_shader_int_value(_M_ID, value_name, value);
        return *this;
    }

    const Shader& Shader::set(const std::string& value_name, const glm::mat4& value) const
    {
        set_shader_mat4_float_value(_M_ID, value_name, value);
        return *this;
    }

    const Shader& Shader::set(const std::string& value_name, const glm::mat3& value) const
    {
        set_shader_mat3_float_value(_M_ID, value_name, value);
        return *this;
    }

    const Shader& Shader::set(const std::string& value_name, const bool& value) const
    {
        set_shader_int_value(_M_ID, value_name, static_cast<int>(value));
        return *this;
    }

    const Shader& Shader::set(const std::string& value_name, const glm::vec2& value) const
    {
        set_shader_vec2_float_value(_M_ID, value_name, value);
        return *this;
    }

    const Shader& Shader::set(const std::string& value_name, const glm::vec3& value) const
    {
        set_shader_vec3_float_value(_M_ID, value_name, value);
        return *this;
    }

    const Shader& Shader::set(const std::string& value_name, const Vector4D& value) const
    {
        set_shader_vec4_float_value(_M_ID, value_name, value);
        return *this;
    }


}// namespace Engine
