#include <Core/object.hpp>
#include <Core/pointer.hpp>

namespace Engine
{
	class SceneComponent;
	class Pipeline;
	struct MaterialParameterInfo;
	class Sampler;
	class Texture2D;
	class Material;
	class RenderPass;

	namespace MaterialParameters
	{
		class ENGINE_EXPORT Parameter : public Object
		{
			declare_class(Parameter, Object);

		protected:
			virtual Parameter& apply(SceneComponent* component, Pipeline* pipeline, RenderPass* render_pass,
									 MaterialParameterInfo* info) = 0;

		public:
			friend class Engine::Material;
		};

		class ENGINE_EXPORT PrimitiveBase : public Parameter
		{
		protected:
			PrimitiveBase& update(const void* data, size_t size, MaterialParameterInfo* info);
			bool serialize_internal(Archive& ar, void* data, size_t size);
		};

		template<typename T>
		class Primitive : public PrimitiveBase
		{
		public:
			T value;

		protected:
			Primitive(const T& value = T()) : value(value)
			{}

			Primitive& apply(SceneComponent* component, Pipeline* pipeline, RenderPass* render_pass,
							 MaterialParameterInfo* info) override
			{
				update(&value, sizeof(T), info);
				return *this;
			}

			bool serialize(Archive& ar) override
			{
				if (!Super::serialize(ar))
					return false;
				return serialize_internal(ar, &value, sizeof(T));
			}
		};

		class ENGINE_EXPORT Bool : public Primitive<bool>
		{
			declare_class(Bool, Parameter);
		};

		class ENGINE_EXPORT Int : public Primitive<int32_t>
		{
			declare_class(Int, Parameter);
		};

		class ENGINE_EXPORT UInt : public Primitive<uint32_t>
		{
			declare_class(UInt, Parameter);
		};

		class ENGINE_EXPORT Float : public Primitive<float>
		{
			declare_class(Float, Parameter);
		};

		class ENGINE_EXPORT Bool2 : public Primitive<Vector2b>
		{
			declare_class(Bool2, Parameter);
		};

		class ENGINE_EXPORT Bool3 : public Primitive<Vector3b>
		{
			declare_class(Bool3, Parameter);
		};

		class ENGINE_EXPORT Bool4 : public Primitive<Vector4b>
		{
			declare_class(Bool2, Parameter);
		};

		class ENGINE_EXPORT Int2 : public Primitive<Vector2i>
		{
			declare_class(Int2, Parameter);
		};

		class ENGINE_EXPORT Int3 : public Primitive<Vector3i>
		{
			declare_class(Int3, Parameter);
		};

		class ENGINE_EXPORT Int4 : public Primitive<Vector4i>
		{
			declare_class(Int4, Parameter);
		};

		class ENGINE_EXPORT UInt2 : public Primitive<Vector2u>
		{
			declare_class(UInt2, Parameter);
		};

		class ENGINE_EXPORT UInt3 : public Primitive<Vector3u>
		{
			declare_class(UInt3, Parameter);
		};

		class ENGINE_EXPORT UInt4 : public Primitive<Vector4u>
		{
			declare_class(UInt4, Parameter);
		};

		class ENGINE_EXPORT Float2 : public Primitive<Vector2f>
		{
			declare_class(Float2, Parameter);
		};

		class ENGINE_EXPORT Float3 : public Primitive<Vector3f>
		{
			declare_class(Float3, Parameter);
		};

		class ENGINE_EXPORT Float4 : public Primitive<Vector4f>
		{
			declare_class(Float4, Parameter);
		};

		class ENGINE_EXPORT Float3x3 : public Primitive<Matrix3f>
		{
			declare_class(Float3x3, Parameter);

		public:
			Float3x3() : Primitive<Matrix3f>(Matrix3f(1.f))
			{}
		};

		class ENGINE_EXPORT Float4x4 : public Primitive<Matrix4f>
		{
			declare_class(Float4x4, Parameter);

		public:
			bool is_model = false;

			Float4x4() : Primitive<Matrix4f>(Matrix3f(1.f))
			{}

			Float4x4& apply(SceneComponent* component, Pipeline* pipeline, RenderPass* render_pass,
							MaterialParameterInfo* info) override;
		};

		class ENGINE_EXPORT Model4x4 : public Parameter
		{
			declare_class(Model4x4, Parameter);

		public:
			Model4x4& apply(SceneComponent* component, Pipeline* pipeline, RenderPass* render_pass,
							MaterialParameterInfo* info) override;
		};

		class ENGINE_EXPORT Sampler : public Parameter
		{
			declare_class(Sampler, Parameter);

		public:
			Engine::Sampler* sampler;

			Sampler();
			Sampler& apply(SceneComponent* component, Pipeline* pipeline, RenderPass* render_pass,
						   MaterialParameterInfo* info) override;
			bool serialize(Archive& ar) override;
		};

		class ENGINE_EXPORT Sampler2D : public Parameter
		{
			declare_class(Sampler2D, Parameter);

		public:
			Engine::Sampler* sampler;
			Engine::Texture2D* texture;

			Sampler2D();
			Sampler2D& apply(SceneComponent* component, Pipeline* pipeline, RenderPass* render_pass,
							 MaterialParameterInfo* info) override;
			bool serialize(Archive& ar) override;
		};

		class ENGINE_EXPORT Texture2D : public Parameter
		{
			declare_class(Texture2D, Parameter);

		public:
			Engine::Texture2D* texture;

			Texture2D();
			Texture2D& apply(SceneComponent* component, Pipeline* pipeline, RenderPass* render_pass,
							 MaterialParameterInfo* info) override;
			bool serialize(Archive& ar) override;
		};

		class ENGINE_EXPORT Globals : public Parameter
		{
			declare_class(Globals, Parameter);

		public:
			Globals& apply(SceneComponent* component, Pipeline* pipeline, RenderPass* render_pass,
						   MaterialParameterInfo* info) override;
		};
	}// namespace MaterialParameters
}// namespace Engine
