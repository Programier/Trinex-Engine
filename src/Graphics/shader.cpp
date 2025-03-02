#include <Core/archive.hpp>
#include <Core/base_engine.hpp>
#include <Core/buffer_manager.hpp>
#include <Core/engine_types.hpp>
#include <Core/file_manager.hpp>
#include <Core/logger.hpp>
#include <Core/name.hpp>
#include <Core/reflection/class.hpp>
#include <Core/reflection/enum.hpp>
#include <Core/reflection/property.hpp>
#include <Core/reflection/struct.hpp>
#include <Graphics/rhi.hpp>
#include <Graphics/scene_render_targets.hpp>
#include <Graphics/shader.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Engine
{
	implement_struct(Engine::VertexShader::Attribute, 0)
	{
		auto self = static_struct_instance();

		Refl::Enum* type_enum     = Refl::Enum::static_require("Engine::VertexBufferElementType");
		Refl::Enum* rate_enum     = Refl::Enum::static_require("Engine::VertexAttributeInputRate");
		Refl::Enum* semantic_enum = Refl::Enum::static_require("Engine::VertexBufferSemantic");

		auto default_flags   = Refl::Property::IsNotSerializable;
		auto read_only_flags = default_flags | Refl::Property::IsReadOnly;

		trinex_refl_prop(self, This, name, read_only_flags)->tooltip("Name of this attribute");
		trinex_refl_prop(self, This, type, type_enum, default_flags)->tooltip("Type of element of this attribute");
		trinex_refl_prop(self, This, rate, rate_enum, default_flags)->tooltip("Rate of this attribute");
		trinex_refl_prop(self, This, semantic, semantic_enum, read_only_flags)->tooltip("Semantic of this attribute");
		trinex_refl_prop(self, This, semantic_index, read_only_flags)->tooltip("Semantic index of this attribute");
		trinex_refl_prop(self, This, location, read_only_flags)->tooltip("Location index of this attribute");
		trinex_refl_prop(self, This, stream_index, default_flags)->tooltip("The stream index from which to read this attribute");
		trinex_refl_prop(self, This, offset, default_flags)->tooltip("Offset of this attribute in vertex struct");
	}

	implement_engine_class_default_init(Shader, 0);

	implement_engine_class(VertexShader, 0)
	{
		auto* self = This::static_class_instance();
		trinex_refl_prop(self, This, attributes, Refl::Property::IsNotSerializable | Refl::Property::IsReadOnly)
				->display_name("Vertex Attributes")
				.tooltip("Vertex attributes of this pipeline");
	}

	implement_engine_class_default_init(TessellationControlShader, 0);
	implement_engine_class_default_init(TessellationShader, 0);
	implement_engine_class_default_init(GeometryShader, 0);
	implement_engine_class_default_init(FragmentShader, 0);

	bool Shader::serialize(Archive& ar)
	{
		if (!Super::serialize(ar))
			return false;
		return ar;
	}

	bool Shader::serialize_source_code(Archive& ar)
	{
		return ar.serialize(source_code);
	}

	VertexShader& VertexShader::rhi_init()
	{
		m_rhi_object.reset(rhi->create_vertex_shader(this));
		return *this;
	}

	bool VertexShader::serialize(Archive& ar)
	{
		if (!Super::serialize(ar))
			return false;

		ar.serialize(attributes);
		return ar;
	}

	ShaderType VertexShader::type() const
	{
		return ShaderType::Vertex;
	}

	FragmentShader& FragmentShader::rhi_init()
	{
		m_rhi_object.reset(rhi->create_fragment_shader(this));
		return *this;
	}

	ShaderType FragmentShader::type() const
	{
		return ShaderType::Fragment;
	}

	TessellationControlShader& TessellationControlShader::rhi_init()
	{
		m_rhi_object.reset(rhi->create_tesselation_control_shader(this));
		return *this;
	}

	ShaderType TessellationControlShader::type() const
	{
		return ShaderType::TessellationControl;
	}

	TessellationShader& TessellationShader::rhi_init()
	{
		m_rhi_object.reset(rhi->create_tesselation_shader(this));
		return *this;
	}

	ShaderType TessellationShader::type() const
	{
		return ShaderType::Tessellation;
	}

	GeometryShader& GeometryShader::rhi_init()
	{
		m_rhi_object.reset(rhi->create_geometry_shader(this));
		return *this;
	}

	ShaderType GeometryShader::type() const
	{
		return ShaderType::Geometry;
	}

	ComputeShader& ComputeShader::rhi_init()
	{
		return *this;
	}

	ShaderType ComputeShader::type() const
	{
		return ShaderType::Compute;
	}

	bool VertexShader::Attribute::serialize(Archive& ar)
	{
		ar.serialize(name);
		ar.serialize(type);
		ar.serialize(semantic);
		ar.serialize(semantic_index);
		ar.serialize(rate);
		ar.serialize(location);
		ar.serialize(stream_index);
		return ar.serialize(offset);
	}

}// namespace Engine
