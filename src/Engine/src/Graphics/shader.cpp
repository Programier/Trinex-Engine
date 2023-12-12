#include <Core/buffer_manager.hpp>
#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Core/engine_config.hpp>
#include <Core/engine_types.hpp>
#include <Core/logger.hpp>
#include <Graphics/scene_render_targets.hpp>
#include <Graphics/render_target_base.hpp>
#include <Graphics/rhi.hpp>
#include <Graphics/shader.hpp>
#include <fstream>
#include <glm/gtc/type_ptr.hpp>
#include <sstream>


namespace Engine
{
    implement_class(Shader, "Engine");
    implement_default_initialize_class(Shader);

    implement_class(VertexShader, "Engine");
    implement_default_initialize_class(VertexShader);

    implement_class(FragmentShader, "Engine");
    implement_default_initialize_class(FragmentShader);


    Shader& Shader::init_global_ubo(BindLocation location)
    {
        global_ubo_location = location;
        uniform_buffers.emplace_back();
        UniformBuffer& ubo = uniform_buffers.back();

        ubo.location = location;
        ubo.name     = "Global";
        ubo.size     = sizeof(RenderTargetBase::GlobalUniforms);

        return *this;
    }


    VertexShader& VertexShader::rhi_create()
    {
        _M_rhi_object.reset(engine_instance->rhi()->create_vertex_shader(this));
        return *this;
    }

    FragmentShader& FragmentShader::rhi_create()
    {
        _M_rhi_object.reset(engine_instance->rhi()->create_fragment_shader(this));
        return *this;
    }
}// namespace Engine
