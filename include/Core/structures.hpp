#pragma once
#include <Core/enums.hpp>
#include <Core/name.hpp>

namespace Engine
{
	struct Rect2D {
		Vector2i pos  = {0, 0};
		Vector2i size = {0, 0};

		FORCE_INLINE Rect2D(Vector2i pos = {0, 0}, Vector2i size = {0, 0}) : pos(pos), size(size) {}
	};

	struct ViewPort {
		Vector2i pos    = {0, 0};
		Vector2i size   = {0, 0};
		float min_depth = 0.0f;
		float max_depth = 1.0f;


		FORCE_INLINE ViewPort(Vector2i pos = {0, 0}, Vector2i size = {0, 0}, float min_depth = 0.f, float max_depth = 1.f)
		    : pos(pos), size(size), min_depth(min_depth), max_depth(max_depth)
		{}

		FORCE_INLINE float aspect() const { return static_cast<float>(size.x) / static_cast<float>(size.y); }

		FORCE_INLINE bool operator==(const ViewPort& v) const
		{
			return pos == v.pos && size == v.size && glm::epsilonEqual(min_depth, v.min_depth, 0.0001f) &&
			       glm::epsilonEqual(max_depth, v.max_depth, 0.0001f);
		}

		FORCE_INLINE bool operator!=(const ViewPort& v) const { return !((*this) == v); }
	};

	struct Scissor {
		Vector2i pos  = {0, 0};
		Vector2i size = {0, 0};

		FORCE_INLINE Scissor(Vector2i pos = {0, 0}, Vector2i size = {0, 0}) : pos(pos), size(size) {}
		FORCE_INLINE bool operator==(const Scissor& v) const { return pos == v.pos && size == v.size; }
		FORCE_INLINE bool operator!=(const Scissor& v) const { return !((*this) == v); }
	};

	struct DepthStencilClearValue {
		float depth  = 1.0;
		byte stencil = 0.0;
	};

	struct ClassFieldInfo {
		const char* name;
		AccessType access;
		bool is_serializable;
	};

	struct EmptyStruct {
	};

	class EmptyClass
	{
	};

	struct ENGINE_EXPORT BindLocation {
		static const BindLocation undefined;

		BindingIndex binding;

		constexpr BindLocation(BindingIndex in_binding = 255) : binding(in_binding) {}

		FORCE_INLINE bool operator==(const BindLocation& location) const { return location.binding == binding; }

		FORCE_INLINE bool operator!=(const BindLocation& location) const { return location.binding != binding; }

		FORCE_INLINE bool operator<(const BindLocation& location) const { return binding < location.binding; }

		FORCE_INLINE bool operator<=(const BindLocation& location) const { return binding <= location.binding; }

		FORCE_INLINE bool operator>(const BindLocation& location) const { return binding > location.binding; }

		FORCE_INLINE bool operator>=(const BindLocation& location) const { return binding >= location.binding; }


		FORCE_INLINE bool is_valid() const { return binding < 255; }

		FORCE_INLINE operator BindingIndex() const { return binding; }
	};

	struct ENGINE_EXPORT ShaderDefinition {
		trinex_declare_struct(ShaderDefinition, void);

		String key;
		String value;

		bool serialize(class Archive& ar);
	};

	class ENGINE_EXPORT MaterialScalarParametersInfo final
	{
		BindingIndex m_binding_index = 255;

	public:
		FORCE_INLINE bool has_parameters() const { return m_binding_index < 255; }

		FORCE_INLINE BindingIndex bind_index() const { return m_binding_index; }

		FORCE_INLINE MaterialScalarParametersInfo& bind_index(BindingIndex index)
		{
			m_binding_index = index;
			return *this;
		}

		FORCE_INLINE MaterialScalarParametersInfo& remove_parameters()
		{
			m_binding_index = 255;
			return *this;
		}
	};

	struct ENGINE_EXPORT ShaderParameterInfo {
		ShaderParameterType type = ShaderParameterType::Undefined;
		Name name;
		size_t size           = 0;
		size_t offset         = 0;
		BindingIndex location = 255;

		bool serialize(Archive& ar);
	};

	struct VertexAttribute {
		Name name;
		VertexBufferElementType type;
		VertexAttributeInputRate rate;
		VertexBufferSemantic semantic;
		byte semantic_index;
		byte location;
		byte stream_index;
		uint16_t offset;

		FORCE_INLINE VertexAttribute(VertexAttributeInputRate rate = VertexAttributeInputRate::Vertex,
		                             VertexBufferSemantic semantic = VertexBufferSemantic::Position, byte semantic_index = 0,
		                             byte location = 0, byte stream = 0, uint16_t offset = 0, const Name& name = Name::none)
		    : name(name), rate(rate), semantic(semantic), semantic_index(semantic_index), location(location), offset(offset)
		{}

		bool serialize(Archive& ar);
	};
}// namespace Engine
