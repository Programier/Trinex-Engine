#pragma once

#include <Core/object.hpp>
#include <Core/pointer.hpp>
#include <Graphics/sampler.hpp>

namespace Engine
{
	class SceneComponent;
	struct ShaderParameterInfo;
	class Texture2D;
	class Material;
	class RenderPass;
	class MaterialInterface;
	class RenderSurface;

	namespace MaterialParameters
	{
#define trinex_material_parameter(self, super)                                                                                   \
	trinex_declare_class(self, super);                                                                                           \
                                                                                                                                 \
public:                                                                                                                          \
	static Engine::ShaderParameterType static_type()                                                                             \
	{                                                                                                                            \
		return Engine::ShaderParameterType::self;                                                                                \
	}                                                                                                                            \
                                                                                                                                 \
	inline Engine::ShaderParameterType type() const override                                                                     \
	{                                                                                                                            \
		return Engine::ShaderParameterType::self;                                                                                \
	}

		class ENGINE_EXPORT Parameter : public Object
		{
			trinex_declare_class(Parameter, Object);

		private:
			uint16_t m_pipeline_refs = 0;

		protected:
			virtual Parameter& apply(SceneComponent* component, RenderPass* render_pass, ShaderParameterInfo* info) = 0;

		public:
			static Refl::Class* static_find_class(ShaderParameterType type);

			virtual ShaderParameterType type() const = 0;
			friend class Engine::Material;
			friend class Engine::MaterialInterface;
		};

		class ENGINE_EXPORT PrimitiveBase : public Parameter
		{
		protected:
			PrimitiveBase& update(const void* data, size_t size, ShaderParameterInfo* info);
			bool serialize_internal(Archive& ar, void* data, size_t size);
		};

		template<typename T>
		class Primitive : public PrimitiveBase
		{
		public:
			T value;

		protected:
			Primitive(const T& value = T()) : value(value) {}

			Primitive& apply(SceneComponent* component, RenderPass* render_pass, ShaderParameterInfo* info) override
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
			trinex_material_parameter(Bool, Parameter);
		};

		class ENGINE_EXPORT Int : public Primitive<int32_t>
		{
			trinex_material_parameter(Int, Parameter);
		};

		class ENGINE_EXPORT UInt : public Primitive<uint32_t>
		{
			trinex_material_parameter(UInt, Parameter);
		};

		class ENGINE_EXPORT Float : public Primitive<float>
		{
			trinex_material_parameter(Float, Parameter);
		};

		class ENGINE_EXPORT Bool2 : public Primitive<Vector2b>
		{
			trinex_material_parameter(Bool2, Parameter);
		};

		class ENGINE_EXPORT Bool3 : public Primitive<Vector3b>
		{
			trinex_material_parameter(Bool3, Parameter);
		};

		class ENGINE_EXPORT Bool4 : public Primitive<Vector4b>
		{
			trinex_material_parameter(Bool2, Parameter);
		};

		class ENGINE_EXPORT Int2 : public Primitive<Vector2i>
		{
			trinex_material_parameter(Int2, Parameter);
		};

		class ENGINE_EXPORT Int3 : public Primitive<Vector3i>
		{
			trinex_material_parameter(Int3, Parameter);
		};

		class ENGINE_EXPORT Int4 : public Primitive<Vector4i>
		{
			trinex_material_parameter(Int4, Parameter);
		};

		class ENGINE_EXPORT UInt2 : public Primitive<Vector2u>
		{
			trinex_material_parameter(UInt2, Parameter);
		};

		class ENGINE_EXPORT UInt3 : public Primitive<Vector3u>
		{
			trinex_material_parameter(UInt3, Parameter);
		};

		class ENGINE_EXPORT UInt4 : public Primitive<Vector4u>
		{
			trinex_material_parameter(UInt4, Parameter);
		};

		class ENGINE_EXPORT Float2 : public Primitive<Vector2f>
		{
			trinex_material_parameter(Float2, Parameter);
		};

		class ENGINE_EXPORT Float3 : public Primitive<Vector3f>
		{
			trinex_material_parameter(Float3, Parameter);
		};

		class ENGINE_EXPORT Float4 : public Primitive<Vector4f>
		{
			trinex_material_parameter(Float4, Parameter);
		};

		class ENGINE_EXPORT Float3x3 : public Primitive<Matrix3f>
		{
			trinex_material_parameter(Float3x3, Parameter);

		public:
			Float3x3() : Primitive<Matrix3f>(Matrix3f(1.f)) {}
		};

		class ENGINE_EXPORT Float4x4 : public Primitive<Matrix4f>
		{
			trinex_material_parameter(Float4x4, Parameter);

		public:
			bool is_model = false;

			Float4x4() : Primitive<Matrix4f>(Matrix3f(1.f)) {}

			Float4x4& apply(SceneComponent* component, RenderPass* render_pass, ShaderParameterInfo* info) override;
		};

		class ENGINE_EXPORT LocalToWorld : public Parameter
		{
			trinex_material_parameter(LocalToWorld, Parameter);

		public:
			LocalToWorld& apply(SceneComponent* component, RenderPass* render_pass, ShaderParameterInfo* info) override;
		};

		class ENGINE_EXPORT Sampler : public Parameter
		{
			trinex_material_parameter(Sampler, Parameter);

		public:
			Engine::Sampler sampler;

			Sampler();
			Sampler& apply(SceneComponent* component, RenderPass* render_pass, ShaderParameterInfo* info) override;
			bool serialize(Archive& ar) override;
		};

		class ENGINE_EXPORT Sampler2D : public Parameter
		{
			trinex_material_parameter(Sampler2D, Parameter);

		public:
			Engine::Sampler sampler;
			Engine::Texture2D* texture;

			Sampler2D();
			Sampler2D& apply(SceneComponent* component, RenderPass* render_pass, ShaderParameterInfo* info) override;
			bool serialize(Archive& ar) override;
		};

		class ENGINE_EXPORT Texture2D : public Parameter
		{
			trinex_material_parameter(Texture2D, Parameter);

		public:
			Engine::Texture2D* texture;

			Texture2D();
			Texture2D& apply(SceneComponent* component, RenderPass* render_pass, ShaderParameterInfo* info) override;
			bool serialize(Archive& ar) override;
		};

		class ENGINE_EXPORT Globals : public Parameter
		{
			trinex_material_parameter(Globals, Parameter);

		public:
			Globals& apply(SceneComponent* component, RenderPass* render_pass, ShaderParameterInfo* info) override;
		};

		class ENGINE_EXPORT Surface : public Parameter
		{
			trinex_material_parameter(Surface, Parameter);

		public:
			Engine::RenderSurface* surface;

			Surface();
			Surface& apply(SceneComponent* component, RenderPass* render_pass, ShaderParameterInfo* info) override;
			bool serialize(Archive& ar) override;
		};

		class ENGINE_EXPORT CombinedSurface : public Parameter
		{
			trinex_material_parameter(CombinedSurface, Parameter);

		public:
			Engine::RenderSurface* surface;
			Engine::Sampler sampler;

			CombinedSurface();
			CombinedSurface& apply(SceneComponent* component, RenderPass* render_pass, ShaderParameterInfo* info) override;
			bool serialize(Archive& ar) override;
		};
	}// namespace MaterialParameters
}// namespace Engine
