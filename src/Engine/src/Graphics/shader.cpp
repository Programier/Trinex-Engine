#include <Core/archive.hpp>
#include <Core/buffer_manager.hpp>
#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Core/engine_config.hpp>
#include <Core/engine_types.hpp>
#include <Core/logger.hpp>
#include <Core/name.hpp>
#include <Graphics/render_target_base.hpp>
#include <Graphics/rhi.hpp>
#include <Graphics/scene_render_targets.hpp>
#include <Graphics/shader.hpp>
#include <fstream>
#include <glm/gtc/type_ptr.hpp>
#include <sstream>

namespace Engine
{
    implement_class(Shader, Engine, 0);
    implement_default_initialize_class(Shader);

    implement_class(VertexShader, Engine, 0);
    implement_default_initialize_class(VertexShader);

    implement_class(FragmentShader, Engine, 0);
    implement_default_initialize_class(FragmentShader);


    bool Shader::archive_process(Archive& ar)
    {
        if (!Super::archive_process(ar))
            return false;

        ar& uniform_buffers;
        ar& samplers;
        ar& textures;
        ar& combined_samplers;
        ar& ssbo;
        ar& text_code;
        ar& binary_code;
        return ar;
    }

    VertexShader& VertexShader::rhi_create()
    {
        _M_rhi_object.reset(engine_instance->rhi()->create_vertex_shader(this));
        return *this;
    }

    bool VertexShader::archive_process(Archive& ar)
    {
        if (!Super::archive_process(ar))
            return false;

        ar& attributes;
        return ar;
    }

    FragmentShader& FragmentShader::rhi_create()
    {
        _M_rhi_object.reset(engine_instance->rhi()->create_fragment_shader(this));
        return *this;
    }


    ENGINE_EXPORT bool operator&(Archive& ar, Shader::UniformBuffer& buffer)
    {
        ar& buffer.name;
        ar& buffer.location.id;
        ar& buffer.size;
        return ar;
    }

    ENGINE_EXPORT bool operator&(Archive& ar, Shader::SSBO& buffer)
    {
        ar& buffer.name;
        ar& buffer.location.id;
        return ar;
    }

    ENGINE_EXPORT bool operator&(Archive& ar, Shader::Texture& texture)
    {
        ar& texture.name;
        ar& texture.location.id;
        return ar;
    }

    ENGINE_EXPORT bool operator&(Archive& ar, VertexShader::Attribute& attrib)
    {
        ar& attrib.name;
        ar& attrib.count;
        ar& attrib.format;
        ar& attrib.rate;
        return ar;
    }

}// namespace Engine
