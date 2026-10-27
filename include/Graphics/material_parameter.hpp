#pragma once

#include <Core/math/matrix.hpp>
#include <Core/math/vector.hpp>
#include <Core/deprecated/object.hpp>
#include <Core/pointer.hpp>
#include <Graphics/sampler.hpp>

namespace Trinex
{
	struct PrimitiveRenderingContext;
	struct RHIShaderParameterInfo;
	class Texture2D;
	class Material;
	class RenderPass;
	class MaterialInterface;
	class RenderSurface;

	namespace MaterialParameters
	{
#define trinex_material_parameter(self, super)                                                                                   \
	trinex_class(self, super);                                                                                                   \
                                                                                                                                 \
public:                                                                                                                          \
	static Trinex::RHIShaderParameterType static_type()                                                                          \
	{                                                                                                                            \
		return Trinex::RHIShaderParameterType::self;                                                                             \
	}                                                                                                                            \
                                                                                                                                 \
	inline Trinex::RHIShaderParameterType type() const override                                                                  \
	{                                                                                                                            \
		return Trinex::RHIShaderParameterType::self;                                                                             \
	}

		class trinex_api Parameter : public DeprecatedObject
		{
			trinex_class(Parameter, DeprecatedObject);

		private:
			u16 m_pipeline_refs = 0;

		protected:
			virtual Parameter& apply(const PrimitiveRenderingContext* ctx, const RHIShaderParameterInfo* info) = 0;

		public:
			static Refl::Class* static_find_class(RHIShaderParameterType type);

			virtual RHIShaderParameterType type() const = 0;
			friend class Trinex::Material;
			friend class Trinex::MaterialInterface;
		};

		class trinex_api PrimitiveBase : public Parameter
		{
		protected:
			PrimitiveBase& update(const PrimitiveRenderingContext* ctx, const void* data, usize size,
			                      const RHIShaderParameterInfo* info);
			bool serialize_internal(Archive& ar, void* data, usize size);
		};

		template<typename T>
		class Primitive : public PrimitiveBase
		{
		public:
			T value;

		protected:
			Primitive(const T& value = T()) : value(value) {}

			Primitive& apply(const PrimitiveRenderingContext* ctx, const RHIShaderParameterInfo* info) override
			{
				update(ctx, &value, sizeof(T), info);
				return *this;
			}

			bool serialize(Archive& ar) override
			{
				if (!Super::serialize(ar))
					return false;
				return serialize_internal(ar, &value, sizeof(T));
			}
		};

		class trinex_api Bool : public Primitive<bool>
		{
			trinex_material_parameter(Bool, Parameter);
		};

		class trinex_api Int : public Primitive<i32>
		{
			trinex_material_parameter(Int, Parameter);
		};

		class trinex_api UInt : public Primitive<u32>
		{
			trinex_material_parameter(UInt, Parameter);
		};

		class trinex_api Float : public Primitive<float>
		{
			trinex_material_parameter(Float, Parameter);
		};

		class trinex_api Bool2 : public Primitive<Vector2b>
		{
			trinex_material_parameter(Bool2, Parameter);
		};

		class trinex_api Bool3 : public Primitive<Vector3b>
		{
			trinex_material_parameter(Bool3, Parameter);
		};

		class trinex_api Bool4 : public Primitive<Vector4b>
		{
			trinex_material_parameter(Bool4, Parameter);
		};

		class trinex_api Int2 : public Primitive<Vector2i>
		{
			trinex_material_parameter(Int2, Parameter);
		};

		class trinex_api Int3 : public Primitive<Vector3i>
		{
			trinex_material_parameter(Int3, Parameter);
		};

		class trinex_api Int4 : public Primitive<Vector4i>
		{
			trinex_material_parameter(Int4, Parameter);
		};

		class trinex_api UInt2 : public Primitive<Vector2u>
		{
			trinex_material_parameter(UInt2, Parameter);
		};

		class trinex_api UInt3 : public Primitive<Vector3u>
		{
			trinex_material_parameter(UInt3, Parameter);
		};

		class trinex_api UInt4 : public Primitive<Vector4u>
		{
			trinex_material_parameter(UInt4, Parameter);
		};

		class trinex_api Float2 : public Primitive<Vector2f>
		{
			trinex_material_parameter(Float2, Parameter);
		};

		class trinex_api Float3 : public Primitive<Vector3f>
		{
			trinex_material_parameter(Float3, Parameter);
		};

		class trinex_api Float4 : public Primitive<Vector4f>
		{
			trinex_material_parameter(Float4, Parameter);
		};

		class trinex_api Float3x3 : public Primitive<Matrix3f>
		{
			trinex_material_parameter(Float3x3, Parameter);

		public:
			Float3x3() : Primitive<Matrix3f>(Matrix3f(1.f)) {}
		};

		class trinex_api Float4x4 : public Primitive<Matrix4f>
		{
			trinex_material_parameter(Float4x4, Parameter);

		public:
			Float4x4() : Primitive<Matrix4f>(Matrix3f(1.f)) {}
			Float4x4& apply(const PrimitiveRenderingContext* ctx, const RHIShaderParameterInfo* info) override;
		};

		class trinex_api LocalToWorld : public Parameter
		{
			trinex_material_parameter(LocalToWorld, Parameter);

		public:
			LocalToWorld& apply(const PrimitiveRenderingContext* ctx, const RHIShaderParameterInfo* info) override;
		};

		class trinex_api Sampler : public Parameter
		{
			trinex_material_parameter(Sampler, Parameter);

		public:
			trinex_property(IsTransient)
			Trinex::Sampler sampler;

			Sampler();
			Sampler& apply(const PrimitiveRenderingContext* ctx, const RHIShaderParameterInfo* info) override;
			bool serialize(Archive& ar) override;
		};

		class trinex_api Sampler2D : public Parameter
		{
			trinex_material_parameter(Sampler2D, Parameter);

		public:
			trinex_property(IsTransient)
			Trinex::Sampler sampler;

			Trinex::Texture2D* texture;

			Sampler2D();
			Sampler2D& apply(const PrimitiveRenderingContext* ctx, const RHIShaderParameterInfo* info) override;
			bool serialize(Archive& ar) override;
		};

		class trinex_api Texture2D : public Parameter
		{
			trinex_material_parameter(Texture2D, Parameter);

		public:
			Trinex::Texture2D* texture;

			Texture2D();
			Texture2D& apply(const PrimitiveRenderingContext* ctx, const RHIShaderParameterInfo* info) override;
			bool serialize(Archive& ar) override;
		};

		class trinex_api Globals : public Parameter
		{
			trinex_material_parameter(Globals, Parameter);

		public:
			Globals& apply(const PrimitiveRenderingContext* ctx, const RHIShaderParameterInfo* info) override;
		};

		class trinex_api Surface : public Parameter
		{
			trinex_material_parameter(Surface, Parameter);

		public:
			Trinex::RenderSurface* surface;

			Surface();
			Surface& apply(const PrimitiveRenderingContext* ctx, const RHIShaderParameterInfo* info) override;
			bool serialize(Archive& ar) override;
		};

		class trinex_api CombinedSurface : public Parameter
		{
			trinex_material_parameter(CombinedSurface, Parameter);

		public:
			Trinex::RenderSurface* surface;

			trinex_reflect(IsTransient)
			Trinex::Sampler sampler;

			CombinedSurface();
			CombinedSurface& apply(const PrimitiveRenderingContext* ctx, const RHIShaderParameterInfo* info) override;
			bool serialize(Archive& ar) override;
		};
	} // namespace MaterialParameters
} // namespace Trinex
