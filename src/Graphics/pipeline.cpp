#include <Core/archive.hpp>
#include <Core/file_manager.hpp>
#include <Core/garbage_collector.hpp>
#include <Core/logger.hpp>
#include <Core/package.hpp>
#include <Core/reflection/class.hpp>
#include <Core/reflection/property.hpp>
#include <Core/threading.hpp>
#include <Engine/Render/render_pass.hpp>
#include <Graphics/material.hpp>
#include <Graphics/pipeline.hpp>
#include <Graphics/shader.hpp>
#include <Graphics/shader_cache.hpp>
#include <Graphics/shader_compiler.hpp>
#include <RHI/rhi.hpp>
#include <RHI/initializers.hpp>

namespace Engine
{
	static FORCE_INLINE bool init_shader(Shader* shader)
	{
		if (shader && !shader->source_code.empty())
		{
			shader->init_render_resources();
			return true;
		}
		return false;
	}

	static FORCE_INLINE void destroy_shader(Shader* shader)
	{
		if (shader)
		{
			GarbageCollector::destroy(shader);
		}
	}

	GraphicsPipeline::~GraphicsPipeline()
	{
		destroy_shader(m_vertex_shader);
		destroy_shader(m_tessellation_control_shader);
		destroy_shader(m_tessellation_shader);
		destroy_shader(m_geometry_shader);
		destroy_shader(m_fragment_shader);
	}

	static RHIShader* extract_shader(Shader* shader)
	{
		return shader ? shader->rhi_shader() : nullptr;
	}

	StringView Pipeline::pipeline_name_of(StringView name)
	{
		return Strings::class_name_sv_of(name);
	}

	Object* Pipeline::package_of(StringView name)
	{
		Package* package         = Package::static_find_package("TrinexEngine::GlobalPipelines", true);
		StringView child_package = Strings::namespace_sv_of(name);

		if (!child_package.empty())
		{
			package = package->find_package(child_package, true);
		}

		return package;
	}

	Shader* Pipeline::create_new_shader()
	{
		Shader* shader = Object::new_instance<Shader>();
		shader->owner(this);
		return shader;
	}

	Pipeline& Pipeline::release_render_resources()
	{
		Super::release_render_resources();
		m_pipeline = nullptr;
		return *this;
	}

	const Pipeline& Pipeline::rhi_bind() const
	{
		if (RHIPipeline* pipeline = m_pipeline)
		{
			pipeline->bind();
		}
		return *this;
	}

	bool Pipeline::serialize(Archive& ar)
	{
		return serialize(ar, nullptr);
	}

	bool Pipeline::serialize(Archive& ar, Material* material)
	{
		if (!Super::serialize(ar))
			return false;
		return true;
	}

	Pipeline& Pipeline::clear()
	{
		m_pipeline = nullptr;
		m_parameters.clear();
		return *this;
	}

	Pipeline& Pipeline::modify_compilation_env(ShaderCompilationEnvironment* env)
	{
		return *this;
	}

	class Material* Pipeline::material() const
	{
		return Object::instance_cast<Material>(owner());
	}

	static bool pipeline_parameter_compare(const RHIShaderParameterInfo& a, const Name& name)
	{
		return a.name < name;
	}

	bool Pipeline::add_parameter(const RHIShaderParameterInfo& parameter, bool replace)
	{
		auto begin = m_parameters.begin();
		auto end   = m_parameters.end();
		auto it    = std::lower_bound(begin, end, parameter.name, pipeline_parameter_compare);

		if (it != end && it->name == parameter.name)
		{
			if (replace)
			{
				(*it) = parameter;
				return true;
			}

			error_log("Pipeline", "Parameter with name '%s' already exist!");
			return false;
		}

		m_parameters.insert(it, parameter);
		return true;
	}

	bool Pipeline::remove_parameter(const Name& name)
	{
		auto begin = m_parameters.begin();
		auto end   = m_parameters.end();
		auto it    = std::lower_bound(begin, end, name, pipeline_parameter_compare);

		if (it != end && it->name == name)
		{
			m_parameters.erase(it);
			return true;
		}
		return false;
	}

	const RHIShaderParameterInfo* Pipeline::find_parameter(const Name& name) const
	{
		auto begin = m_parameters.begin();
		auto end   = m_parameters.end();
		auto it    = std::lower_bound(begin, end, name, pipeline_parameter_compare);
		return it == end ? nullptr : it;
	}

	Pipeline& Pipeline::parameters(const Vector<RHIShaderParameterInfo>& parameters)
	{
		using Info   = RHIShaderParameterInfo;
		m_parameters = parameters;
		std::sort(m_parameters.begin(), m_parameters.end(), [](const Info& a, const Info& b) { return a.name < b.name; });
		return *this;
	}

	GraphicsPipeline& GraphicsPipeline::init_render_resources()
	{
		const bool vs_inited = init_shader(m_vertex_shader);
		init_shader(m_tessellation_control_shader);
		init_shader(m_tessellation_shader);
		init_shader(m_geometry_shader);
		const bool fs_inited = init_shader(m_fragment_shader);


		if (vs_inited && fs_inited)
		{
			render_thread()->call([this]() {
				RHIGraphicsPipelineInitializer initializer;
				initializer.vertex_shader               = extract_shader(vertex_shader());
				initializer.tessellation_control_shader = extract_shader(tessellation_control_shader());
				initializer.tessellation_shader         = extract_shader(tessellation_shader());
				initializer.geometry_shader             = extract_shader(geometry_shader());
				initializer.fragment_shader             = extract_shader(fragment_shader());
				initializer.parameters                  = parameters().data();
				initializer.parameters_count            = parameters().size();
				initializer.vertex_attributes           = vertex_attributes.data();
				initializer.vertex_attributes_count     = vertex_attributes.size();

				initializer.parameters       = parameters().data();
				initializer.parameters_count = parameters().size();

				initializer.depth    = depth_test;
				initializer.stencil  = stencil_test;
				initializer.blending = color_blending;

				m_pipeline = rhi->create_graphics_pipeline(&initializer);
			});
		}
		return *this;
	}

	GraphicsPipeline& GraphicsPipeline::postload()
	{
		// Initialize shaders first!
		Super::postload();
		return *this;
	}

	Shader* GraphicsPipeline::vertex_shader() const
	{
		return m_vertex_shader;
	}

	Shader* GraphicsPipeline::fragment_shader() const
	{
		return m_fragment_shader;
	}

	Shader* GraphicsPipeline::tessellation_control_shader() const
	{
		return m_tessellation_control_shader;
	}

	Shader* GraphicsPipeline::tessellation_shader() const
	{
		return m_tessellation_shader;
	}

	Shader* GraphicsPipeline::geometry_shader() const
	{
		return m_geometry_shader;
	}

	Shader* GraphicsPipeline::vertex_shader(bool create)
	{
		if (!m_vertex_shader && create)
		{
			m_vertex_shader = create_new_shader();
		}

		return m_vertex_shader;
	}

	Shader* GraphicsPipeline::fragment_shader(bool create)
	{
		if (!m_fragment_shader && create)
		{
			m_fragment_shader = create_new_shader();
		}

		return m_fragment_shader;
	}

	Shader* GraphicsPipeline::tessellation_control_shader(bool create)
	{
		if (!m_tessellation_control_shader && create)
		{
			m_tessellation_control_shader = create_new_shader();
		}

		return m_tessellation_control_shader;
	}

	Shader* GraphicsPipeline::tessellation_shader(bool create)
	{
		if (!m_tessellation_shader && create)
		{
			m_tessellation_shader = create_new_shader();
		}

		return m_tessellation_shader;
	}

	Shader* GraphicsPipeline::geometry_shader(bool create)
	{
		if (!m_geometry_shader && create)
		{
			m_geometry_shader = create_new_shader();
		}

		return m_geometry_shader;
	}

	GraphicsPipeline& GraphicsPipeline::remove_vertex_shader()
	{
		if (m_vertex_shader)
		{
			GarbageCollector::destroy(m_vertex_shader);
			m_vertex_shader = nullptr;
		}
		return *this;
	}

	GraphicsPipeline& GraphicsPipeline::remove_fragment_shader()
	{
		if (m_fragment_shader)
		{
			GarbageCollector::destroy(m_fragment_shader);
			m_fragment_shader = nullptr;
		}
		return *this;
	}

	GraphicsPipeline& GraphicsPipeline::remove_tessellation_control_shader()
	{
		if (m_tessellation_control_shader)
		{
			GarbageCollector::destroy(m_tessellation_control_shader);
			m_tessellation_control_shader = nullptr;
		}
		return *this;
	}

	GraphicsPipeline& GraphicsPipeline::remove_tessellation_shader()
	{
		if (m_tessellation_shader)
		{
			GarbageCollector::destroy(m_tessellation_shader);
			m_tessellation_shader = nullptr;
		}
		return *this;
	}

	GraphicsPipeline& GraphicsPipeline::remove_geometry_shader()
	{
		if (m_geometry_shader)
		{
			GarbageCollector::destroy(m_geometry_shader);
			m_geometry_shader = nullptr;
		}
		return *this;
	}

	bool GraphicsPipeline::serialize(class Archive& archive, Material* material)
	{
		static auto serialize_shader_internal = [](Shader* shader, Archive& archive) {
			if (shader)
			{
				shader->serialize(archive);
			}
		};

		static auto compile_pipeline = [](Material* material, GraphicsPipeline* pipeline) -> bool {
			RenderPass* pass = RenderPass::static_find(pipeline->name());
			if (pass == nullptr)
			{
				error_log("Pipeline", "Failed to find render pass with name '%s'", pipeline->name().c_str());
				return false;
			}

			return material->compile(nullptr, pass);
		};

		if (!Super::serialize(archive, material))
			return false;

		if (!material)
		{
			archive.serialize(depth_test);
			archive.serialize(stencil_test);
			archive.serialize(color_blending);
		}

		vertex_shader(true);
		fragment_shader(true);

		serialize_shader_internal(m_vertex_shader, archive);
		serialize_shader_internal(m_tessellation_control_shader, archive);
		serialize_shader_internal(m_tessellation_shader, archive);
		serialize_shader_internal(m_geometry_shader, archive);
		serialize_shader_internal(m_fragment_shader, archive);

		// Loading shaders from shader cache
		String pipeline_name = full_name();

		GraphicsShaderCache cache;

		bool cache_serialize_result = false;

		if (archive.is_reading())
		{
			cache_serialize_result = cache.load(pipeline_name);
		}
		else
		{
			cache.init_from(this);
			cache_serialize_result = cache.store(pipeline_name);
		}

		if (!cache_serialize_result && archive.is_reading())
		{
			warn_log("GraphicsPipeline", "Missing shader cache for pipeline '%s'.%s", pipeline_name.c_str(),
			         material ? " Recompiling..." : "");

			if (material)
			{
				if ((cache_serialize_result = compile_pipeline(material, this)))
				{
					info_log("GraphicsPipeline", "Compile success!");

					cache.init_from(this);
					cache.store(pipeline_name);
				}
				else
				{
					error_log("GraphicsPipeline", "Compile fail!");
				}
			}
		}
		else if (cache_serialize_result && archive.is_reading())
		{
			cache.apply_to(this);
		}

		return archive && cache_serialize_result;
	}

	ComputePipeline::ComputePipeline()
	{
		m_shader = create_new_shader();
	}

	ComputePipeline::~ComputePipeline()
	{
		destroy_shader(m_shader);
	}

	ComputePipeline& ComputePipeline::init_render_resources()
	{
		init_shader(m_shader);
		render_thread()->call([this]() {
			RHIComputePipelineInitializer initializer;
			initializer.compute_shader   = compute_shader()->rhi_shader();
			initializer.parameters       = parameters().data();
			initializer.parameters_count = parameters().size();

			m_pipeline = rhi->create_compute_pipeline(&initializer);
		});
		return *this;
	}

	GlobalGraphicsPipeline::GlobalGraphicsPipeline(StringView name) {}

	template<typename ShaderCacheType, typename Type>
	static Type& load_global_pipeline(Type* self)
	{
		ShaderCacheType cache;
		String cache_path = self->full_name();

		if (!cache.load(cache_path))
		{
			String source_code;
			FileReader reader(self->shader_path());

			if (reader.is_open())
			{
				source_code = reader.read_string();
			}
			else
			{
				throw EngineException("Failed to read global shader");
			}

			ShaderCompiler::StackEnvironment env;
			ShaderCompilationResult result;

			env.add_source(source_code.c_str());
			self->modify_compilation_env(&env);

			if (ShaderCompiler::instance()->compile(&env, result))
			{
				cache.init_from(result);
				cache.store(cache_path);
			}
			else
			{
				throw EngineException("Failed to compile global shader");
			}
		}

		cache.apply_to(self);

		self->initialize();
		self->init_render_resources();
		return *self;
	}

	GlobalGraphicsPipeline& GlobalGraphicsPipeline::load_pipeline()
	{
		return load_global_pipeline<GraphicsShaderCache>(this);
	}

	bool GlobalGraphicsPipeline::serialize(Archive& ar, Material* material)
	{
		return ar;
	}

	GlobalComputePipeline::GlobalComputePipeline(StringView name) {}

	GlobalComputePipeline& GlobalComputePipeline::load_pipeline()
	{
		return load_global_pipeline<ComputeShaderCache>(this);
	}

	bool GlobalComputePipeline::serialize(Archive& ar, Material* material)
	{
		return ar;
	}

	trinex_implement_engine_class_default_init(Pipeline, 0);
	trinex_implement_engine_class_default_init(GraphicsPipeline, 0);
	trinex_implement_engine_class_default_init(ComputePipeline, 0);

}// namespace Engine
