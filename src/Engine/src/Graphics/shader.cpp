#include <Core/archive.hpp>
#include <Core/buffer_manager.hpp>
#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Core/engine_config.hpp>
#include <Core/engine_types.hpp>
#include <Core/file_manager.hpp>
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

    implement_class(TessellationControlShader, Engine, 0);
    implement_default_initialize_class(TessellationControlShader);

    implement_class(TessellationShader, Engine, 0);
    implement_default_initialize_class(TessellationShader);

    implement_class(GeometryShader, Engine, 0);
    implement_default_initialize_class(GeometryShader);

    implement_class(FragmentShader, Engine, 0);
    implement_default_initialize_class(FragmentShader);

    bool Shader::archive_process(Archive& ar)
    {
        if (!Super::archive_process(ar))
            return false;

        ar & samplers;
        ar & combined_image_samplers;
        ar & textures;
        ar & ssbo;
        return ar;
    }

    bool Shader::archive_process_source_code(Archive& ar)
    {
        return ar & source_code;
    }

    Shader& Shader::clean()
    {
        samplers.clear();
        textures.clear();
        ssbo.clear();
        return *this;
    }

    VertexShader& VertexShader::rhi_create()
    {
        m_rhi_object.reset(engine_instance->rhi()->create_vertex_shader(this));
        return *this;
    }

    bool VertexShader::archive_process(Archive& ar)
    {
        if (!Super::archive_process(ar))
            return false;

        ar & attributes;
        return ar;
    }

    ShaderType VertexShader::type() const
    {
        return ShaderType::Vertex;
    }

    FragmentShader& FragmentShader::rhi_create()
    {
        m_rhi_object.reset(engine_instance->rhi()->create_fragment_shader(this));
        return *this;
    }

    ShaderType FragmentShader::type() const
    {
        return ShaderType::Fragment;
    }

    TessellationControlShader& TessellationControlShader::rhi_create()
    {
        m_rhi_object.reset(engine_instance->rhi()->create_tesselation_control_shader(this));
        return *this;
    }

    ShaderType TessellationControlShader::type() const
    {
        return ShaderType::TessellationControl;
    }

    TessellationShader& TessellationShader::rhi_create()
    {
        m_rhi_object.reset(engine_instance->rhi()->create_tesselation_shader(this));
        return *this;
    }

    ShaderType TessellationShader::type() const
    {
        return ShaderType::Tessellation;
    }

    GeometryShader& GeometryShader::rhi_create()
    {
        m_rhi_object.reset(engine_instance->rhi()->create_geometry_shader(this));
        return *this;
    }

    ShaderType GeometryShader::type() const
    {
        return ShaderType::Geometry;
    }

    ENGINE_EXPORT bool operator&(Archive& ar, Shader::SSBO& buffer)
    {
        ar & buffer.name;
        ar & buffer.location.id;
        return ar;
    }

    ENGINE_EXPORT bool operator&(Archive& ar, Shader::Texture& texture)
    {
        ar & texture.name;
        ar & texture.location.id;
        return ar;
    }

    ENGINE_EXPORT bool operator&(Archive& ar, Shader::Sampler& object)
    {
        ar & object.name;
        ar & object.location.id;
        return ar;
    }

    ENGINE_EXPORT bool operator&(Archive& ar, Shader::CombinedImageSampler& object)
    {
        ar & object.name;
        ar & object.location.id;
        return ar;
    }


    ENGINE_EXPORT bool operator&(Archive& ar, VertexShader::Attribute& attrib)
    {
        ar & attrib.name;
        ar & attrib.count;
        ar & attrib.format;
        ar & attrib.semantic;
        ar & attrib.semantic_index;
        ar & attrib.rate;
        return ar;
    }

}// namespace Engine
