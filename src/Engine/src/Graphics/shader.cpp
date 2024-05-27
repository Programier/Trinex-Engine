#include <Core/archive.hpp>
#include <Core/buffer_manager.hpp>
#include <Core/class.hpp>
#include <Core/base_engine.hpp>
#include <Core/engine_types.hpp>
#include <Core/enum.hpp>
#include <Core/file_manager.hpp>
#include <Core/logger.hpp>
#include <Core/name.hpp>
#include <Core/property.hpp>
#include <Graphics/render_target_base.hpp>
#include <Graphics/rhi.hpp>
#include <Graphics/scene_render_targets.hpp>
#include <Graphics/shader.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Engine
{
    implement_struct(Attribute, Engine::VertexShader, ).push([]() {
        Struct* self                      = Struct::static_find("Engine::VertexShader::Attribute", true);
        Enum* type_enum                   = Enum::static_find("Engine::VertexBufferElementType", true);
        Enum* vertex_attribute_rate_enum  = Enum::static_find("Engine::VertexAttributeInputRate", true);
        Enum* vertex_buffer_semantic_enum = Enum::static_find("Engine::VertexBufferSemantic", true);

        self->add_property(new NameProperty("Name", "Name of this attribute", &VertexShader::Attribute::name, Name::none,
                                            Property::IsConst | Property::IsNotSerializable));
        self->add_property(new EnumProperty("Element Type", "Type of element of this attribute", &VertexShader::Attribute::type,
                                            type_enum, Name::none, Property::IsNotSerializable));
        self->add_property(new EnumProperty("Rate", "Rate of this attribute", &VertexShader::Attribute::rate,
                                            vertex_attribute_rate_enum, Name::none, Property::IsNotSerializable));
        self->add_property(new EnumProperty("Semantic", "Semantic of this attribute", &VertexShader::Attribute::semantic,
                                            vertex_buffer_semantic_enum, Name::none,
                                            Property::IsConst | Property::IsNotSerializable));
        self->add_property(new ByteProperty("Count", "Count of elements for this attribute",
                                            &VertexShader::Attribute::semantic_index, Name::none,
                                            Property::IsConst | Property::IsNotSerializable));

        self->add_property(new ByteProperty("Semantic Index", "Semantic index of this attribute",
                                            &VertexShader::Attribute::semantic_index, Name::none,
                                            Property::IsConst | Property::IsNotSerializable));
    });

    implement_class(Shader, Engine, 0);
    implement_default_initialize_class(Shader);

    static Name get_name_of_attribute(class ArrayPropertyInterface* interface, void* object, size_t index)
    {
        if (index >= interface->elements_count(object))
        {
            return Name::out_of_range;
        }

        VertexShader::Attribute* attribute = reinterpret_cast<VertexShader::Attribute*>(interface->at(object, index));
        return attribute->name;
    }

    implement_class(VertexShader, Engine, 0);
    implement_initialize_class(VertexShader)
    {
        Class* self              = This::static_class_instance();
        Struct* attribute_struct = Struct::static_find("Engine::VertexShader::Attribute", true);

        auto attributes_prop =
                new StructProperty<This, Attribute>("", "", nullptr, attribute_struct, Name::none, Property::IsNotSerializable);
        auto attributes_array_prop =
                new ArrayProperty("Vertex Attributes", "Vertex attributes of this pipeline", &This::attributes, attributes_prop,
                                  Name::none, Property::IsConst | Property::IsNotSerializable);
        attributes_array_prop->element_name_callback(get_name_of_attribute);
        self->add_property(attributes_array_prop);
    }

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
        return ar;
    }

    bool Shader::archive_process_source_code(Archive& ar)
    {
        return ar & source_code;
    }

    VertexShader& VertexShader::rhi_create()
    {
        m_rhi_object.reset(rhi->create_vertex_shader(this));
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
        m_rhi_object.reset(rhi->create_fragment_shader(this));
        return *this;
    }

    ShaderType FragmentShader::type() const
    {
        return ShaderType::Fragment;
    }

    TessellationControlShader& TessellationControlShader::rhi_create()
    {
        m_rhi_object.reset(rhi->create_tesselation_control_shader(this));
        return *this;
    }

    ShaderType TessellationControlShader::type() const
    {
        return ShaderType::TessellationControl;
    }

    TessellationShader& TessellationShader::rhi_create()
    {
        m_rhi_object.reset(rhi->create_tesselation_shader(this));
        return *this;
    }

    ShaderType TessellationShader::type() const
    {
        return ShaderType::Tessellation;
    }

    GeometryShader& GeometryShader::rhi_create()
    {
        m_rhi_object.reset(rhi->create_geometry_shader(this));
        return *this;
    }

    ShaderType GeometryShader::type() const
    {
        return ShaderType::Geometry;
    }

    ENGINE_EXPORT bool operator&(Archive& ar, VertexShader::Attribute& attrib)
    {
        ar & attrib.name;
        ar & attrib.type;
        ar & attrib.semantic;
        ar & attrib.semantic_index;
        ar & attrib.rate;
        return ar;
    }

}// namespace Engine
