#pragma once
#include <Core/enums.hpp>
#include <Core/etl/string.hpp>

namespace Engine
{
	struct Rect2D {
		Point2D position = {0.f, 0.f};
		Size2D size      = {0.f, 0.f};
	};

	struct ViewPort {
		Point2D pos = {0.0f, 0.0f};
		Size2D size;
		float min_depth = 0.0f;
		float max_depth = 1.0f;


		FORCE_INLINE float aspect() const
		{
			return size.x / size.y;
		}

		FORCE_INLINE bool operator==(const ViewPort& v) const
		{
			return glm::all(glm::epsilonEqual(pos, v.pos, Point2D{0.001f, 0.0001f})) &&
			       glm::all(glm::epsilonEqual(size, v.size, Size2D{0.001f, 0.0001f})) &&
			       glm::epsilonEqual(min_depth, v.min_depth, 0.0001f) && glm::epsilonEqual(max_depth, v.max_depth, 0.0001f);
		}

		FORCE_INLINE bool operator!=(const ViewPort& v) const
		{
			return !((*this) == v);
		}
	};

	struct Scissor {
		Point2D pos = {0.0f, 0.0f};
		Size2D size = {0.f, 0.f};

		FORCE_INLINE bool operator==(const Scissor& v) const
		{
			return glm::all(glm::epsilonEqual(pos, v.pos, Point2D{0.001f, 0.0001f})) &&
			       glm::all(glm::epsilonEqual(size, v.size, Size2D{0.001f, 0.0001f}));
		}

		FORCE_INLINE bool operator!=(const Scissor& v) const
		{
			return !((*this) == v);
		}
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

		constexpr BindLocation(BindingIndex in_binding = 255) : binding(in_binding)
		{}

		FORCE_INLINE bool operator==(const BindLocation& location) const
		{
			return location.binding == binding;
		}

		FORCE_INLINE bool operator!=(const BindLocation& location) const
		{
			return location.binding != binding;
		}

		FORCE_INLINE bool operator<(const BindLocation& location) const
		{
			return binding < location.binding;
		}

		FORCE_INLINE bool operator<=(const BindLocation& location) const
		{
			return binding <= location.binding;
		}

		FORCE_INLINE bool operator>(const BindLocation& location) const
		{
			return binding > location.binding;
		}

		FORCE_INLINE bool operator>=(const BindLocation& location) const
		{
			return binding >= location.binding;
		}


		FORCE_INLINE bool is_valid() const
		{
			return binding < 255;
		}

		FORCE_INLINE operator BindingIndex() const
		{
			return binding;
		}
	};

	struct ENGINE_EXPORT ShaderDefinition {
		declare_struct(ShaderDefinition, void);

		String key;
		String value;

		bool serialize(class Archive& ar);
	};

	class ENGINE_EXPORT MaterialScalarParametersInfo final
	{
		BindingIndex m_binding_index = 255;

	public:
		FORCE_INLINE bool has_parameters() const
		{
			return m_binding_index < 255;
		}

		FORCE_INLINE BindingIndex bind_index() const
		{
			return m_binding_index;
		}

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

	struct ENGINE_EXPORT MaterialParameterInfo {
		class Refl::Class* type = nullptr;
		String name;
		size_t size;
		size_t offset;
		BindingIndex location;

		MaterialParameterInfo();
		bool serialize(Archive& ar);
	};
}// namespace Engine
