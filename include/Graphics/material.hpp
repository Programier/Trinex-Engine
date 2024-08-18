#pragma once
#include <Core/default_resources.hpp>
#include <Core/object.hpp>
#include <Core/pointer.hpp>
#include <Core/structures.hpp>
#include <Graphics/sampler.hpp>
#include <Graphics/texture.hpp>
#include <Graphics/texture_2D.hpp>

namespace Engine
{
	namespace ShaderCompiler
	{
		class Compiler;
		struct ShaderSource;
	}// namespace ShaderCompiler

	class Pipeline;
	class SceneComponent;
	class Texture2D;
	class Sampler;

	class ENGINE_EXPORT MaterialParameter : public SerializableObject
	{
	public:
		Name name;

	protected:
		static bool serialize_data(Archive& ar, void* data, size_t size);

	public:
		virtual size_t size() const;
		virtual byte* data();
		virtual const byte* data() const;
		virtual MaterialParameterType type() const = 0;
		virtual MaterialParameterType binding_object_type() const;
		virtual MaterialParameter& apply(const Pipeline* pipeline, SceneComponent* component);

		template<typename T>
		T* get()
		{
			if (size() == sizeof(T))
			{
				return reinterpret_cast<T*>(data());
			}
			return nullptr;
		}

		template<typename T>
		const T* get() const
		{
			if (size() == sizeof(T))
			{
				return reinterpret_cast<const T*>(data());
			}
			return nullptr;
		}

		virtual ~MaterialParameter() = default;
	};

	template<typename TypeData, MaterialParameterType _parameter_type>
	class TypedMaterialParameter : public MaterialParameter
	{
	public:
		TypeData param;

		static constexpr inline MaterialParameterType parameter_type = _parameter_type;
		TypedMaterialParameter(TypeData value = TypeData()) : param(value)
		{}


		MaterialParameterType type() const override
		{
			return parameter_type;
		}

		size_t size() const override
		{
			return sizeof(TypeData);
		}

		byte* data() override
		{
			return reinterpret_cast<byte*>(&param);
		}

		const byte* data() const override
		{
			return reinterpret_cast<const byte*>(&param);
		}

		bool archive_process(Archive& ar) override
		{
			if (!MaterialParameter::archive_process(ar))
				return false;
			return serialize_data(ar, &param, sizeof(param));
		}
	};

	template<typename TypeData, MaterialParameterType _parameter_type>
	class TypedMatrixMaterialParameter : public TypedMaterialParameter<TypeData, _parameter_type>
	{
	public:
		TypedMatrixMaterialParameter(TypeData value = TypeData(1.f))
			: TypedMaterialParameter<TypeData, _parameter_type>(value)
		{}
	};

	using BoolMaterialParameter	 = TypedMaterialParameter<bool, MaterialParameterType::Bool>;
	using IntMaterialParameter	 = TypedMaterialParameter<int32_t, MaterialParameterType::Int>;
	using UIntMaterialParameter	 = TypedMaterialParameter<uint32_t, MaterialParameterType::UInt>;
	using FloatMaterialParameter = TypedMaterialParameter<float, MaterialParameterType::Float>;

	using BVec2MaterialParameter = TypedMaterialParameter<BoolVector2D, MaterialParameterType::BVec2>;
	using BVec3MaterialParameter = TypedMaterialParameter<BoolVector3D, MaterialParameterType::BVec3>;
	using BVec4MaterialParameter = TypedMaterialParameter<BoolVector4D, MaterialParameterType::BVec4>;

	using IVec2MaterialParameter = TypedMaterialParameter<IntVector2D, MaterialParameterType::IVec2>;
	using IVec3MaterialParameter = TypedMaterialParameter<IntVector3D, MaterialParameterType::IVec3>;
	using IVec4MaterialParameter = TypedMaterialParameter<IntVector4D, MaterialParameterType::IVec4>;

	using UVec2MaterialParameter = TypedMaterialParameter<UIntVector2D, MaterialParameterType::UVec2>;
	using UVec3MaterialParameter = TypedMaterialParameter<UIntVector3D, MaterialParameterType::UVec3>;
	using UVec4MaterialParameter = TypedMaterialParameter<UIntVector4D, MaterialParameterType::UVec4>;

	using Vec2MaterialParameter = TypedMaterialParameter<Vector2D, MaterialParameterType::Vec2>;
	using Vec3MaterialParameter = TypedMaterialParameter<Vector3D, MaterialParameterType::Vec3>;
	using Vec4MaterialParameter = TypedMaterialParameter<Vector4D, MaterialParameterType::Vec4>;
	using Mat3MaterialParameter = TypedMatrixMaterialParameter<Matrix3f, MaterialParameterType::Mat3>;

	struct ENGINE_EXPORT Mat4MaterialParameter
		: public TypedMatrixMaterialParameter<Matrix4f, MaterialParameterType::Mat4> {
		using Super = TypedMatrixMaterialParameter<Matrix4f, MaterialParameterType::Mat4>;

		bool bind_model_matrix = false;

		Mat4MaterialParameter& apply(const Pipeline* pipeline, SceneComponent* component) override;
		bool archive_process(Archive& archive) override;
	};

	class ENGINE_EXPORT BindingMaterialParameter : public MaterialParameter
	{
	protected:
		void bind_texture(class Engine::Texture* texture, const Pipeline* pipeline);
		void bind_sampler(class Engine::Sampler* sampler, const Pipeline* pipeline);
		bool archive_process(Archive& ar) override;
		static bool archive_status(Archive& ar);

	public:
		virtual class Texture* texture_param() const;
		virtual class Sampler* sampler_param() const;
		virtual BindingMaterialParameter& texture_param(class Texture* texture);
		virtual BindingMaterialParameter& sampler_param(class Sampler* sampler);
	};

	class ENGINE_EXPORT SamplerMaterialParameter : public BindingMaterialParameter
	{
	public:
		Pointer<class Sampler> sampler;

		SamplerMaterialParameter();
		MaterialParameterType type() const override;
		MaterialParameter& apply(const Pipeline* pipeline, SceneComponent* component = nullptr) override;
		bool archive_process(Archive& ar) override;
		class Sampler* sampler_param() const override;
		SamplerMaterialParameter& sampler_param(class Sampler* sampler) override;
	};

	struct ENGINE_EXPORT TextureMaterialParameterBase : public BindingMaterialParameter {
	public:
		TextureMaterialParameterBase& apply(const Pipeline* pipeline, SceneComponent* component = nullptr) override;
	};

	struct ENGINE_EXPORT CombinedImageSamplerMaterialParameterBase : public BindingMaterialParameter {
	public:
		CombinedImageSamplerMaterialParameterBase& apply(const Pipeline* pipeline,
														 SceneComponent* component = nullptr) override;
	};

	template<typename ClassType, MaterialParameterType parameter_type>
	struct TextureMaterialParameter : public TextureMaterialParameterBase {
		Pointer<ClassType> texture;

		inline MaterialParameterType type() const override
		{
			return parameter_type;
		}

		inline class Texture* texture_param() const override
		{
			return texture.ptr();
		}

		inline TextureMaterialParameter& texture_param(class Texture* in_texture) override
		{
			if (in_texture == nullptr)
			{
				texture = nullptr;
			}
			else if (ClassType* new_texture = Object::instance_cast<ClassType>(in_texture))
			{
				texture = new_texture;
			}
			return *this;
		}

		inline bool archive_process(Archive& ar) override
		{
			if (!BindingMaterialParameter::archive_process(ar))
				return false;

			texture.archive_process(ar, true);
			return archive_status(ar);
		}
	};

	using Texture2DMaterialParameter = TextureMaterialParameter<class Texture2D, MaterialParameterType::Texture2D>;

	template<typename ClassType, MaterialParameterType parameter_type>
	struct CombinedImageSamplerMaterialParameter : public CombinedImageSamplerMaterialParameterBase {
		Pointer<ClassType> texture;
		Pointer<class Sampler> sampler;

		CombinedImageSamplerMaterialParameter() : texture(nullptr), sampler(DefaultResources::Samplers::default_sampler)
		{}

		inline MaterialParameterType type() const override
		{
			return parameter_type;
		}

		inline class Texture* texture_param() const override
		{
			Texture* result = reinterpret_cast<Texture*>(texture.ptr());
			return result;
		}

		inline CombinedImageSamplerMaterialParameter& texture_param(class Texture* in_texture) override
		{
			if (in_texture == nullptr)
			{
				texture = nullptr;
			}
			else if (ClassType* new_texture = Object::instance_cast<ClassType>(in_texture))
			{
				texture = new_texture;
			}
			return *this;
		}

		inline class Sampler* sampler_param() const override
		{
			return sampler.ptr();
		}

		inline CombinedImageSamplerMaterialParameter& sampler_param(class Sampler* in_sampler) override
		{
			sampler = in_sampler;
			return *this;
		}

		inline bool archive_process(Archive& ar) override
		{
			if (!BindingMaterialParameter::archive_process(ar))
				return false;

			texture.archive_process(ar, true);
			sampler.archive_process(ar, true);
			return archive_status(ar);
		}
	};

	struct CombinedImageSampler2DMaterialParameter
		: public CombinedImageSamplerMaterialParameter<class Texture2D, MaterialParameterType::CombinedImageSampler2D> {
		CombinedImageSampler2DMaterialParameter()
		{
			texture = DefaultResources::Textures::default_texture;
		}
	};


	class ENGINE_EXPORT MaterialInterface : public Object
	{
		declare_class(MaterialInterface, Object);

	protected:
		bool serialize_parameters(Map<Name, MaterialParameter*, Name::HashFunction>& map, Archive& ar);

		virtual MaterialParameter* create_parameter_internal(const Name& name, MaterialParameterType type) = 0;

	public:
		virtual MaterialParameter* find_parameter(const Name& name) const;
		virtual MaterialInterface* parent() const;
		virtual class Material* material();
		virtual bool apply(SceneComponent* component = nullptr);
	};

	class ENGINE_EXPORT Material : public MaterialInterface
	{
		declare_class(Material, MaterialInterface);

	public:
		using ParametersMap = Map<Name, MaterialParameter*, Name::HashFunction>;

	private:
		ParametersMap m_material_parameters;

	protected:
		MaterialParameter* create_parameter_internal(const Name& name, MaterialParameterType type) override;

	public:
		Pipeline* pipeline;
		Vector<ShaderDefinition> compile_definitions;

		Material();
		bool archive_process(Archive& archive) override;
		Material& preload() override;
		Material& postload() override;

		MaterialParameter* find_parameter(const Name& name) const override;
		MaterialParameter* create_parameter(const Name& name, MaterialParameterType type);
		Material& remove_parameter(const Name& name);
		Material& clear_parameters();
		const ParametersMap& parameters() const;

		bool apply(SceneComponent* component = nullptr) override;
		bool apply(MaterialInterface* head, SceneComponent* component = nullptr);

		class Material* material() override;
		virtual bool compile(ShaderCompiler::Compiler* compiler = nullptr, MessageList* errors = nullptr);
		Material& apply_changes() override;

		bool submit_compiled_source(const ShaderCompiler::ShaderSource& source, MessageList& errors);

		virtual bool shader_source(String& out_source) = 0;
		~Material();
	};

	class ENGINE_EXPORT MaterialInstance : public MaterialInterface
	{
		declare_class(MaterialInstance, MaterialInterface);

	private:
		Map<Name, MaterialParameter*, Name::HashFunction> m_material_parameters;

	protected:
		MaterialParameter* create_parameter_internal(const Name& name, MaterialParameterType type) override;

	public:
		MaterialInterface* parent_material = nullptr;

		bool archive_process(Archive& archive) override;
		MaterialParameter* find_parameter(const Name& name) const override;
		MaterialInterface* parent() const override;
		bool apply(SceneComponent* component = nullptr) override;
		class Material* material() override;
	};
}// namespace Engine
