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
#include <RHI/rhi_initializers.hpp>

namespace Engine
{
	GraphicsPipeline::~GraphicsPipeline()
	{
		GraphicsPipeline::remove_shaders(ShaderType::All);
	}

	static FORCE_INLINE bool init_shader(Shader* shader)
	{
		if (shader)
		{
			shader->init_render_resources();
			return true;
		}
		return false;
	}

	static RHI_Shader* extract_shader(Shader* shader)
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

	Pipeline& Pipeline::release_render_resources()
	{
		Super::release_render_resources();
		m_pipeline = nullptr;
		return *this;
	}

	const Pipeline& Pipeline::rhi_bind() const
	{
		if (RHI_Pipeline* pipeline = m_pipeline)
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
		remove_shaders(ShaderType::All);
		return *this;
	}

	Pipeline& Pipeline::modify_compilation_env(ShaderCompilationEnvironment* env)
	{
		return *this;
	}

	Pipeline& Pipeline::post_compile(RenderPass* pass)
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

	Pipeline* Pipeline::static_create_pipeline(Type type)
	{
		switch (type)
		{
			case Graphics: return new_instance<GraphicsPipeline>();
			default: return nullptr;
		}
	}

	GraphicsPipeline& GraphicsPipeline::init_render_resources()
	{
		init_shader(m_vertex_shader);
		init_shader(m_tessellation_control_shader);
		init_shader(m_tessellation_shader);
		init_shader(m_geometry_shader);
		init_shader(m_fragment_shader);

		render_thread()->call([this]() {
			RHIGraphicsPipelineInitializer initializer;
			initializer.vertex_shader               = extract_shader(vertex_shader());
			initializer.tessellation_control_shader = extract_shader(tessellation_control_shader());
			initializer.tessellation_shader         = extract_shader(tessellation_shader());
			initializer.geometry_shader             = extract_shader(geometry_shader());
			initializer.fragment_shader             = extract_shader(fragment_shader());
			initializer.parameters                  = parameters().data();
			initializer.parameters_count            = parameters().size();

			initializer.parameters       = parameters().data();
			initializer.parameters_count = parameters().size();

			initializer.depth    = depth_test;
			initializer.stencil  = stencil_test;
			initializer.blending = color_blending;

			m_pipeline = rhi->create_graphics_pipeline(&initializer);
		});
		return *this;
	}

	GraphicsPipeline& GraphicsPipeline::postload()
	{
		// Initialize shaders first!
		Super::postload();
		return *this;
	}

	Shader* GraphicsPipeline::shader(ShaderType type) const
	{
		switch (type)
		{
			case ShaderType::Vertex: return m_vertex_shader;
			case ShaderType::TessellationControl: return m_tessellation_control_shader;
			case ShaderType::Tessellation: return m_tessellation_shader;
			case ShaderType::Geometry: return m_geometry_shader;
			case ShaderType::Fragment: return m_fragment_shader;
			case ShaderType::Compute: return nullptr;
			default: return nullptr;
		}
	}

	Shader* GraphicsPipeline::shader(ShaderType type, bool allocate)
	{
		if (allocate)
		{
			allocate_shaders(Flags<ShaderType>(type));
		}
		return static_cast<const GraphicsPipeline*>(this)->shader(type);
	}

	VertexShader* GraphicsPipeline::vertex_shader() const
	{
		return m_vertex_shader;
	}

	FragmentShader* GraphicsPipeline::fragment_shader() const
	{
		return m_fragment_shader;
	}

	TessellationControlShader* GraphicsPipeline::tessellation_control_shader() const
	{
		return m_tessellation_control_shader;
	}

	TessellationShader* GraphicsPipeline::tessellation_shader() const
	{
		return m_tessellation_shader;
	}

	GeometryShader* GraphicsPipeline::geometry_shader() const
	{
		return m_geometry_shader;
	}


	VertexShader* GraphicsPipeline::vertex_shader(bool create)
	{
		if (!m_vertex_shader && create)
		{
			allocate_shaders(ShaderType::Vertex);
		}

		return m_vertex_shader;
	}

	FragmentShader* GraphicsPipeline::fragment_shader(bool create)
	{
		if (!m_fragment_shader && create)
		{
			allocate_shaders(ShaderType::Fragment);
		}

		return m_fragment_shader;
	}

	TessellationControlShader* GraphicsPipeline::tessellation_control_shader(bool create)
	{
		if (!m_tessellation_control_shader && create)
		{
			allocate_shaders(ShaderType::TessellationControl);
		}

		return m_tessellation_control_shader;
	}

	TessellationShader* GraphicsPipeline::tessellation_shader(bool create)
	{
		if (!m_tessellation_shader && create)
		{
			allocate_shaders(ShaderType::Tessellation);
		}

		return m_tessellation_shader;
	}

	GeometryShader* GraphicsPipeline::geometry_shader(bool create)
	{
		if (!m_geometry_shader && create)
		{
			allocate_shaders(ShaderType::Geometry);
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

	ShaderType GraphicsPipeline::shader_types() const
	{
		ShaderType result(ShaderType::Vertex | ShaderType::Fragment);

		if (m_vertex_shader)
			result |= ShaderType::Vertex;

		if (m_fragment_shader)
			result |= ShaderType::Fragment;

		if (m_tessellation_control_shader)
			result |= ShaderType::TessellationControl;
		if (m_tessellation_shader)
			result |= ShaderType::Tessellation;
		if (m_geometry_shader)
			result |= ShaderType::Geometry;

		return result;
	}

	GraphicsPipeline& GraphicsPipeline::allocate_shaders(ShaderType flags)
	{
		if ((flags & ShaderType::Vertex) == ShaderType::Vertex)
		{
			create_new_shader<VertexShader>("Vertex Shader", m_vertex_shader);
		}

		if ((flags & ShaderType::Fragment) == ShaderType::Fragment)
		{
			create_new_shader<FragmentShader>("Fragment Shader", m_fragment_shader);
		}

		if ((flags & ShaderType::TessellationControl) == ShaderType::TessellationControl)
		{
			create_new_shader<TessellationControlShader>("Tessellation Control Shader", m_tessellation_control_shader);
		}

		if ((flags & ShaderType::Tessellation) == ShaderType::Tessellation)
		{
			create_new_shader<TessellationShader>("Tessellation Shader", m_tessellation_shader);
		}

		if ((flags & ShaderType::Geometry) == ShaderType::Geometry)
		{
			create_new_shader<GeometryShader>("Geometry Shader", m_geometry_shader);
		}

		return *this;
	}

	GraphicsPipeline& GraphicsPipeline::remove_shaders(ShaderType flags)
	{
		if ((flags & ShaderType::Vertex) == ShaderType::Vertex)
		{
			remove_vertex_shader();
		}

		if ((flags & ShaderType::Fragment) == ShaderType::Fragment)
		{
			remove_fragment_shader();
		}

		if ((flags & ShaderType::TessellationControl) == ShaderType::TessellationControl)
		{
			remove_tessellation_control_shader();
		}

		if ((flags & ShaderType::Tessellation) == ShaderType::Tessellation)
		{
			remove_tessellation_shader();
		}

		if ((flags & ShaderType::Geometry) == ShaderType::Geometry)
		{
			remove_geometry_shader();
		}
		return *this;
	}

	Pipeline::Type GraphicsPipeline::type() const
	{
		return Pipeline::Type::Graphics;
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
			ShaderCompiler* compiler = ShaderCompiler::instance();

			if (compiler == nullptr)
			{
				error_log("GraphicsPipeline", "Failed to get material compiler");
				return false;
			}

			RenderPass* pass = RenderPass::static_find(pipeline->name());
			return compiler->compile_pass(material, pass);
		};

		if (!Super::serialize(archive, material))
			return false;

		if (!material)
		{
			archive.serialize(depth_test);
			archive.serialize(stencil_test);
			archive.serialize(color_blending);
		}

		Flags<ShaderType> flags = shader_types();

		archive.serialize(flags);
		allocate_shaders(flags);

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

	ComputePipeline::~ComputePipeline()
	{
		remove_shaders(ShaderType::Compute);
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

	Shader* ComputePipeline::shader(ShaderType type) const
	{
		if ((type & ShaderType::Compute) == ShaderType::Compute)
			return m_shader;
		return nullptr;
	}

	Shader* ComputePipeline::shader(ShaderType type, bool create)
	{
		if ((type & ShaderType::Compute) == ShaderType::Compute)
		{
			if (m_shader == nullptr)
			{
				m_shader = create_new_shader<ComputeShader>("Compute", m_shader);
			}
			return m_shader;
		}
		return nullptr;
	}

	ShaderType ComputePipeline::shader_types() const
	{
		return m_shader ? ShaderType::Compute : ShaderType::Undefined;
	}

	ComputePipeline& ComputePipeline::allocate_shaders(ShaderType type)
	{
		if ((type & ShaderType::Compute) == ShaderType::Compute)
		{
			if (m_shader == nullptr)
			{
				m_shader = create_new_shader<ComputeShader>("Compute", m_shader);
			}
		}
		return *this;
	}

	ComputePipeline& ComputePipeline::remove_shaders(ShaderType type)
	{
		if ((type & ShaderType::Compute) == ShaderType::Compute)
		{
			if (m_shader)
			{
				GarbageCollector::destroy(m_shader);
				m_shader = nullptr;
			}
		}
		return *this;
	}

	Pipeline::Type ComputePipeline::type() const
	{
		return Pipeline::Type::Compute;
	}

	GlobalGraphicsPipeline::GlobalGraphicsPipeline(StringView name, ShaderType types)
	{
		allocate_shaders(types);
	}

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

			if (ShaderCompiler::instance()->compile(source_code, self))
			{
				cache.init_from(self);
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

	GlobalComputePipeline::GlobalComputePipeline(StringView name, ShaderType types)
	{
		allocate_shaders(types);
	}

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
