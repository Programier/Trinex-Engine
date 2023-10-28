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
    implement_class(Shader, "Engine");
    implement_default_initialize_class(Shader);

    implement_class(VertexShader, "Engine");
    implement_default_initialize_class(VertexShader);

    implement_class(FragmentShader, "Engine");
    implement_default_initialize_class(FragmentShader);


    VertexShader& VertexShader::rhi_create()
    {
        rhi_destroy();
        _M_rhi_shader = engine_instance->rhi()->create_vertex_shader(this);
        return *this;
    }

    FragmentShader& FragmentShader::rhi_create()
    {
        rhi_destroy();
        _M_rhi_shader = engine_instance->rhi()->create_fragment_shader(this);
        return *this;
    }

    size_t Shader::stride_of(ShaderDataType type)
    {
        size_t stride = static_cast<size_t>(type);
        return stride >> 25;
    }

    EnumerateType Shader::color_format_of(ShaderDataType type)
    {
        EnumerateType color_index = (static_cast<EnumerateType>(type) >> 18) & 127;
        return static_cast<EnumerateType>(ColorFormatInfo::all_formats()[color_index]);
    }
}// namespace Engine
