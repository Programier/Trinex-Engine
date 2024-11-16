#include <Core/archive.hpp>
#include <Core/base_engine.hpp>
#include <Core/constants.hpp>
#include <Core/file_manager.hpp>
#include <Core/filesystem/root_filesystem.hpp>
#include <Core/logger.hpp>
#include <Core/reflection/class.hpp>
#include <Core/reflection/enum.hpp>
#include <Core/reflection/property.hpp>
#include <Core/reflection/struct.hpp>
#include <Core/threading.hpp>
#include <Engine/project.hpp>
#include <Engine/settings.hpp>
#include <Graphics/material.hpp>
#include <Graphics/pipeline.hpp>
#include <Graphics/pipeline_buffers.hpp>
#include <Graphics/rhi.hpp>
#include <Graphics/shader.hpp>
#include <Graphics/shader_cache.hpp>
#include <Graphics/shader_compiler.hpp>

namespace Engine
{
	implement_struct(Engine::Pipeline::DepthTestInfo, 0)
	{
		auto* self = static_struct_instance();

		trinex_refl_prop(self, This, func, Refl::Enum::static_require("Engine::CompareFunc"))->tooltip("Depth compare function");
		trinex_refl_prop(self, This, enable)->tooltip("Enable depth test");
		trinex_refl_prop(self, This, write_enable)->tooltip("Enable write to depth buffer");
	}

	implement_struct(Engine::Pipeline::StencilTestInfo, 0)
	{
		auto* self                    = static_struct_instance();
		Refl::Enum* stencil_op_enum   = Refl::Enum::static_require("Engine::StencilOp", Refl::FindFlags::IsRequired);
		Refl::Enum* compare_func_enum = Refl::Enum::static_require("Engine::CompareFunc", Refl::FindFlags::IsRequired);

		trinex_refl_prop(self, This, enable)->tooltip("Enable stencil test");
		trinex_refl_prop(self, This, fail, stencil_op_enum)->tooltip("Operation on fail");
		trinex_refl_prop(self, This, depth_pass, stencil_op_enum)->tooltip("Operation on depth pass");
		trinex_refl_prop(self, This, depth_fail, stencil_op_enum)->tooltip("Operation on depth fail");
		trinex_refl_prop(self, This, compare, compare_func_enum)
				->display_name("Compare func")
				.tooltip("Stencil compare function");
		trinex_refl_prop(self, This, compare_mask)->tooltip("Stencil compare mask");
		trinex_refl_prop(self, This, write_mask)->tooltip("Stencil write mask");
	}

	implement_struct(Engine::Pipeline::AssemblyInfo, 0)
	{
		auto* self = static_struct_instance();
		trinex_refl_prop(self, This, primitive_topology, Refl::Enum::static_require("Engine::PrimitiveTopology"))
				->tooltip("Primitive types which will be rendered by this pipeline");
	}

	implement_struct(Engine::Pipeline::RasterizerInfo, 0)
	{
		auto* self = static_struct_instance();

		trinex_refl_prop(self, This, polygon_mode, Refl::Enum::static_require("Engine::PolygonMode"));
		trinex_refl_prop(self, This, cull_mode, Refl::Enum::static_require("Engine::CullMode"));
		trinex_refl_prop(self, This, front_face, Refl::Enum::static_require("Engine::FrontFace"));
		trinex_refl_prop(self, This, line_width);
	}

	implement_struct(Engine::Pipeline::ColorBlendingInfo, 0)
	{
		auto* self = static_struct_instance();

		auto* blend_func = Refl::Enum::static_require("Engine::BlendFunc", Refl::FindFlags::IsRequired);
		auto* blend_op   = Refl::Enum::static_require("Engine::BlendOp", Refl::FindFlags::IsRequired);

		trinex_refl_prop(self, This, enable);
		trinex_refl_prop(self, This, src_color_func, blend_func);
		trinex_refl_prop(self, This, dst_color_func, blend_func);
		trinex_refl_prop(self, This, color_op, blend_op)->display_name("Color Operator");

		trinex_refl_prop(self, This, src_alpha_func, blend_func);
		trinex_refl_prop(self, This, dst_alpha_func, blend_func);
		trinex_refl_prop(self, This, alpha_op, blend_op)->display_name("Alpha Operator");
		trinex_refl_prop(self, This, color_mask, Refl::Enum::static_require("Engine::ColorComponentMask"));
	}

	Pipeline::Pipeline()
	{}

	Pipeline::~Pipeline()
	{
		remove_all_shaders();
	}

#define init_shader(sdr)                                                                                                         \
	if (sdr)                                                                                                                     \
	sdr->rhi_create()

	Pipeline& Pipeline::rhi_create()
	{
		init_shader(m_vertex_shader);
		init_shader(m_tessellation_control_shader);
		init_shader(m_tessellation_shader);
		init_shader(m_geometry_shader);
		init_shader(m_fragment_shader);
		m_rhi_object.reset(rhi->create_pipeline(this));
		return *this;
	}

	Pipeline& Pipeline::postload()
	{
		// Initialize shaders first!
		Super::postload();

		return *this;
	}

	const Pipeline& Pipeline::rhi_bind() const
	{
		if (m_rhi_object)
		{
			rhi_object<RHI_Pipeline>()->bind();
		}
		return *this;
	}

	class Material* Pipeline::material() const
	{
		return Object::instance_cast<Material>(owner());
	}

	VertexShader* Pipeline::vertex_shader() const
	{
		return m_vertex_shader;
	}

	FragmentShader* Pipeline::fragment_shader() const
	{
		return m_fragment_shader;
	}

	TessellationControlShader* Pipeline::tessellation_control_shader() const
	{
		return m_tessellation_control_shader;
	}

	TessellationShader* Pipeline::tessellation_shader() const
	{
		return m_tessellation_shader;
	}

	GeometryShader* Pipeline::geometry_shader() const
	{
		return m_geometry_shader;
	}


	VertexShader* Pipeline::vertex_shader(bool create)
	{
		if (!m_vertex_shader && create)
		{
			allocate_shaders(ShaderType::Vertex);
		}

		return m_vertex_shader;
	}

	FragmentShader* Pipeline::fragment_shader(bool create)
	{
		if (!m_fragment_shader && create)
		{
			allocate_shaders(ShaderType::Fragment);
		}

		return m_fragment_shader;
	}

	TessellationControlShader* Pipeline::tessellation_control_shader(bool create)
	{
		if (!m_tessellation_control_shader && create)
		{
			allocate_shaders(ShaderType::TessellationControl);
		}

		return m_tessellation_control_shader;
	}

	TessellationShader* Pipeline::tessellation_shader(bool create)
	{
		if (!m_tessellation_shader && create)
		{
			allocate_shaders(ShaderType::Tessellation);
		}

		return m_tessellation_shader;
	}

	GeometryShader* Pipeline::geometry_shader(bool create)
	{
		if (!m_geometry_shader && create)
		{
			allocate_shaders(ShaderType::Geometry);
		}

		return m_geometry_shader;
	}

	Pipeline& Pipeline::remove_vertex_shader()
	{
		if (m_vertex_shader)
		{
			delete m_vertex_shader;
			m_vertex_shader = nullptr;
		}
		return *this;
	}

	Pipeline& Pipeline::remove_fragment_shader()
	{
		if (m_fragment_shader)
		{
			delete m_fragment_shader;
			m_fragment_shader = nullptr;
		}
		return *this;
	}

	Pipeline& Pipeline::remove_tessellation_control_shader()
	{
		if (m_tessellation_control_shader)
		{
			delete m_tessellation_control_shader;
			m_tessellation_control_shader = nullptr;
		}
		return *this;
	}

	Pipeline& Pipeline::remove_tessellation_shader()
	{
		if (m_tessellation_shader)
		{
			delete m_tessellation_shader;
			m_tessellation_shader = nullptr;
		}
		return *this;
	}

	Pipeline& Pipeline::remove_geometry_shader()
	{
		if (m_geometry_shader)
		{
			delete m_geometry_shader;
			m_geometry_shader = nullptr;
		}
		return *this;
	}

	Flags<ShaderType> Pipeline::shader_type_flags() const
	{
		Flags<ShaderType> result = Flags(ShaderType::Vertex) | Flags(ShaderType::Fragment);

		if (m_tessellation_control_shader)
			result(ShaderType::TessellationControl, true);
		if (m_tessellation_shader)
			result(ShaderType::Tessellation, true);
		if (m_geometry_shader)
			result(ShaderType::Geometry, true);

		return result;
	}

	Pipeline& Pipeline::allocate_shaders(Flags<ShaderType> flags)
	{
		if (flags & ShaderType::Vertex)
		{
			create_new_shader<VertexShader>("Vertex Shader", m_vertex_shader);
		}

		if (flags & ShaderType::Fragment)
		{
			create_new_shader<FragmentShader>("Fragment Shader", m_fragment_shader);
		}

		if (flags & ShaderType::TessellationControl)
		{
			create_new_shader<TessellationControlShader>("Tessellation Control Shader", m_tessellation_control_shader);
		}

		if (flags & ShaderType::Tessellation)
		{
			create_new_shader<TessellationShader>("Tessellation Shader", m_tessellation_shader);
		}

		if (flags & ShaderType::Geometry)
		{
			create_new_shader<GeometryShader>("Geometry Shader", m_geometry_shader);
		}

		return *this;
	}

	Pipeline& Pipeline::remove_shaders(Flags<ShaderType> flags)
	{
		if (flags & ShaderType::Vertex)
		{
			remove_vertex_shader();
		}

		if (flags & ShaderType::Fragment)
		{
			remove_fragment_shader();
		}

		if (flags & ShaderType::TessellationControl)
		{
			remove_tessellation_control_shader();
		}

		if (flags & ShaderType::Tessellation)
		{
			remove_tessellation_shader();
		}

		if (flags & ShaderType::Geometry)
		{
			remove_geometry_shader();
		}
		return *this;
	}

	Pipeline::ShadersArray Pipeline::shader_array() const
	{
		return {
		        m_vertex_shader,              //
		        m_tessellation_control_shader,//
		        m_tessellation_shader,        //
		        m_geometry_shader,            //
		        m_fragment_shader,            //
		        nullptr                       //
		};
	}

	const MaterialParameterInfo* Pipeline::find_param_info(const Name& name) const
	{
		auto it = parameters.find(name);
		if (it == parameters.end())
			return nullptr;
		return &it->second;
	}

	static FORCE_INLINE bool is_equal_attribute(const VertexShader::Attribute& attr1,
	                                            const ShaderCompiler::ShaderReflection::VertexAttribute& attr2)
	{
		return attr1.type == attr2.type && attr1.semantic == attr2.semantic && attr1.semantic_index == attr2.semantic_index &&
		       attr1.location == attr2.location;
	}

	bool Pipeline::submit_compiled_source(const ShaderCompiler::ShaderSource& source, MessageList& errors)
	{
		bool status = false;

		bool has_valid_graphical_pipeline = source.has_valid_graphical_pipeline();
		bool has_valid_compute_pipiline   = source.has_valid_compute_pipeline();

		Vector<VertexShader::Attribute> vertex_attributes;

		if (has_valid_graphical_pipeline || has_valid_compute_pipiline)
		{
			if (auto shader = vertex_shader())
			{
				vertex_attributes = std::move(shader->attributes);
			}

			remove_all_shaders();
			parameters.clear();
			global_parameters.remove_parameters();
			local_parameters.remove_parameters();
		}
		else
		{
			return false;
		}

		if (has_valid_graphical_pipeline)
		{
			render_thread()->wait_all();

			auto v_shader = vertex_shader(true);
			auto f_shader = fragment_shader(true);

			v_shader->attributes = std::move(vertex_attributes);

			TreeSet<Name> names_to_remove;

			for (auto& entry : v_shader->attributes)
			{
				names_to_remove.insert(entry.name);
			}

			for (auto& attribute : source.reflection.attributes)
			{
				bool next = false;
				for (auto& current_attribute : v_shader->attributes)
				{
					if (current_attribute.name == attribute.name)
					{
						names_to_remove.erase(current_attribute.name);
						next = true;

						if (is_equal_attribute(current_attribute, attribute))
						{
							break;
						}

						current_attribute.type           = attribute.type;
						current_attribute.rate           = attribute.rate;
						current_attribute.semantic       = attribute.semantic;
						current_attribute.semantic_index = attribute.semantic_index;
						current_attribute.location       = attribute.location;
						current_attribute.stream_index   = attribute.stream_index;
						current_attribute.offset         = attribute.offset;
						break;
					}
				}

				if (next)
					continue;

				VertexShader::Attribute out_attribute;
				out_attribute.name           = attribute.name;
				out_attribute.type           = attribute.type;
				out_attribute.rate           = attribute.rate;
				out_attribute.semantic       = attribute.semantic;
				out_attribute.semantic_index = attribute.semantic_index;
				out_attribute.location       = attribute.location;
				out_attribute.stream_index   = attribute.stream_index;
				out_attribute.offset         = attribute.offset;

				v_shader->attributes.push_back(out_attribute);
			}

			auto predicate = [&](const VertexShader::Attribute& attribute) -> bool {
				return names_to_remove.contains(attribute.name);
			};
			auto erase_from = std::remove_if(v_shader->attributes.begin(), v_shader->attributes.end(), predicate);
			v_shader->attributes.erase(erase_from, v_shader->attributes.end());


			v_shader->source_code = source.vertex_code;
			f_shader->source_code = source.fragment_code;

			if (source.has_tessellation_control_shader())
			{
				tessellation_control_shader(true)->source_code = source.tessellation_control_code;
			}
			else
			{
				remove_tessellation_control_shader();
			}

			if (source.has_tessellation_shader())
			{
				tessellation_shader(true)->source_code = source.tessellation_code;
			}
			else
			{
				remove_tessellation_shader();
			}

			if (source.has_geometry_shader())
			{
				geometry_shader(true)->source_code = source.geometry_code;
			}
			else
			{
				remove_geometry_shader();
			}

			for (auto& parameter : source.reflection.uniform_member_infos)
			{
				parameters[parameter.name] = parameter;
			}

			global_parameters = source.reflection.global_parameters_info;
			local_parameters  = source.reflection.local_parameters_info;
			status            = true;
		}

		return status;
	}

	size_t Pipeline::stages_count() const
	{
		size_t count = 0;

		for (auto& ell : shader_array())
		{
			if (ell)
			{
				++count;
			}
		}
		return count;
	}

	bool Pipeline::serialize(class Archive& archive)
	{
		static auto serialize_shader_internal = [](Shader* shader, Archive& archive) {
			if (shader)
			{
				shader->serialize(archive);
			}
		};

		Material* material_object = material();
		if (material_object == nullptr)
		{
			error_log("Pipeline", "Cannot serialize pipeline! Pipeline must be child of material!");
			return false;
		}

		if (!Super::serialize(archive))
			return false;

		auto flags = shader_type_flags();

		archive & flags;
		allocate_shaders(flags);

		serialize_shader_internal(m_vertex_shader, archive);
		serialize_shader_internal(m_tessellation_control_shader, archive);
		serialize_shader_internal(m_tessellation_shader, archive);
		serialize_shader_internal(m_geometry_shader, archive);
		serialize_shader_internal(m_fragment_shader, archive);

		// Loading shaders from shader cache
		String material_name = material_object->full_name(true);

		ShaderCache cache;

		bool cache_serialize_result = false;

		if (archive.is_reading())
		{
			cache_serialize_result = cache.load(material_name);
		}
		else
		{
			cache.init_from(this);
			cache_serialize_result = cache.store(material_name);
		}


		if (!cache_serialize_result && archive.is_reading())
		{
			warn_log("Pipeline", "Missing shader cache for material '%s'. Recompiling...", material_name.c_str());

			if ((cache_serialize_result = material_object->compile()))
			{
				info_log("Pipeline", "Compile success!");

				cache.init_from(this);
				cache.store(material_name);
			}
			else
			{
				error_log("Pipeline", "Compile fail!");
			}
		}
		else if (cache_serialize_result && archive.is_reading())
		{
			cache.apply_to(this);
		}

		return archive && cache_serialize_result;
	}

	implement_engine_class(Pipeline, 0)
	{
		auto* self = static_class_instance();

		trinex_refl_prop(self, This, m_vertex_shader, Refl::Property::IsNotSerializable)->is_composite(true);
		trinex_refl_prop(self, This, depth_test);
		trinex_refl_prop(self, This, stencil_test);
		trinex_refl_prop(self, This, input_assembly);
		trinex_refl_prop(self, This, rasterizer);
		trinex_refl_prop(self, This, color_blending);
	}
}// namespace Engine
