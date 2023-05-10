#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Core/engine_types.hpp>
#include <Core/logger.hpp>
#include <Graphics/shader.hpp>
#include <api.hpp>
#include <fstream>
#include <glm/gtc/type_ptr.hpp>
#include <sstream>



namespace Engine
{
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
