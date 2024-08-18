#pragma once
#include <Core/engine_types.hpp>
#include <Core/etl/templates.hpp>

namespace Engine
{
	enum class MaterialScalarType
	{
		Undefined = 0,
		Bool	  = BIT(0),
		Int8	  = BIT(1),
		Int16	  = BIT(2),
		Int32	  = BIT(3),
		Int64	  = BIT(4),
		UInt8	  = BIT(5),
		UInt16	  = BIT(6),
		UInt32	  = BIT(7),
		UInt64	  = BIT(8),
		Float16	  = BIT(9),
		Float32	  = BIT(10),
		Float64	  = BIT(11)
	};

	struct ENGINE_EXPORT MaterialParameterTypeLayout {
		static inline constexpr size_t scalar_type_size = 12;
		static inline constexpr size_t rows_size		= 4;
		static inline constexpr size_t columns_size		= 4;
		static inline constexpr size_t elements_size	= 4;
		static inline constexpr size_t is_scalar_size	= 1;
		static inline constexpr size_t other_type_size	= 7;

		static inline constexpr size_t scalar_type_offset = 0;
		static inline constexpr size_t rows_offset		  = scalar_type_offset + scalar_type_size;
		static inline constexpr size_t columns_offset	  = rows_offset + rows_size;
		static inline constexpr size_t elements_offset	  = columns_offset + columns_size;
		static inline constexpr size_t is_scalar_offset	  = elements_offset + elements_size;
		static inline constexpr size_t other_type_offset  = is_scalar_offset + is_scalar_size;

		MaterialScalarType scalar_type : scalar_type_size;
		byte rows : rows_size;
		byte columns : columns_size;
		byte elements : elements_size;
		bool is_scalar : is_scalar_size;
		byte other_type : other_type_size;

		FORCE_INLINE constexpr MaterialParameterTypeLayout(MaterialScalarType scalar_type = MaterialScalarType::Bool,
														   byte rows = 0, byte columns = 0, byte elements = 0,
														   bool is_scalar = false, byte other_type = 0)
			: scalar_type(scalar_type), rows(rows), columns(columns), elements(elements), is_scalar(is_scalar),
			  other_type(other_type)
		{}

		FORCE_INLINE static constexpr MaterialParameterTypeLayout from(EnumerateType value)
		{
			auto scalar_type = static_cast<MaterialScalarType>(value & fill_bits<EnumerateType>(scalar_type_size));
			auto rows		 = static_cast<byte>((value >> rows_offset) & fill_bits<byte>(rows_size));
			auto columns	 = static_cast<byte>((value >> columns_offset) & fill_bits<byte>(columns_size));
			auto elements	 = static_cast<byte>((value >> elements_offset) & fill_bits<byte>(elements_size));
			auto is_scalar	 = static_cast<bool>((value >> is_scalar_offset) & fill_bits<byte>(is_scalar_size));
			auto other_type	 = static_cast<byte>((value >> other_type_offset) & fill_bits<byte>(other_type_size));
			return MaterialParameterTypeLayout(scalar_type, rows, columns, elements, is_scalar, other_type);
		}

		template<typename OutType = uint32_t>
		FORCE_INLINE constexpr OutType as_value() const
		{
			size_t value = static_cast<size_t>(scalar_type);
			value |= (static_cast<size_t>(rows) << rows_offset);
			value |= (static_cast<size_t>(columns) << columns_offset);
			value |= (static_cast<size_t>(elements) << elements_offset);
			value |= (static_cast<size_t>(is_scalar) << is_scalar_offset);
			value |= (static_cast<size_t>(other_type) << other_type_offset);
			return static_cast<OutType>(value);
		}
	};

	enum class MaterialParameterType : EnumerateType
	{
		Undefined = 0,

		Bool				   = MaterialParameterTypeLayout(MaterialScalarType::Bool, 1, 1, 0, 1).as_value<>(),
		Int					   = MaterialParameterTypeLayout(MaterialScalarType::Int32, 1, 1, 0, 1).as_value<>(),
		UInt				   = MaterialParameterTypeLayout(MaterialScalarType::UInt32, 1, 1, 0, 1).as_value<>(),
		Float				   = MaterialParameterTypeLayout(MaterialScalarType::Float32, 1, 1, 0, 1).as_value<>(),
		BVec2				   = MaterialParameterTypeLayout(MaterialScalarType::Bool, 1, 2, 2, 1).as_value<>(),
		BVec3				   = MaterialParameterTypeLayout(MaterialScalarType::Bool, 1, 3, 3, 1).as_value<>(),
		BVec4				   = MaterialParameterTypeLayout(MaterialScalarType::Bool, 1, 4, 4, 1).as_value<>(),
		IVec2				   = MaterialParameterTypeLayout(MaterialScalarType::Int32, 1, 2, 2, 1).as_value<>(),
		IVec3				   = MaterialParameterTypeLayout(MaterialScalarType::Int32, 1, 3, 3, 1).as_value<>(),
		IVec4				   = MaterialParameterTypeLayout(MaterialScalarType::Int32, 1, 4, 4, 1).as_value<>(),
		UVec2				   = MaterialParameterTypeLayout(MaterialScalarType::UInt32, 1, 2, 2, 1).as_value<>(),
		UVec3				   = MaterialParameterTypeLayout(MaterialScalarType::UInt32, 1, 3, 3, 1).as_value<>(),
		UVec4				   = MaterialParameterTypeLayout(MaterialScalarType::UInt32, 1, 4, 4, 1).as_value<>(),
		Vec2				   = MaterialParameterTypeLayout(MaterialScalarType::Float32, 1, 2, 2, 1).as_value<>(),
		Vec3				   = MaterialParameterTypeLayout(MaterialScalarType::Float32, 1, 3, 3, 1).as_value<>(),
		Vec4				   = MaterialParameterTypeLayout(MaterialScalarType::Float32, 1, 4, 4, 1).as_value<>(),
		Mat3				   = MaterialParameterTypeLayout(MaterialScalarType::Float32, 3, 3, 0, 1).as_value<>(),
		Mat4				   = MaterialParameterTypeLayout(MaterialScalarType::Float32, 4, 4, 0, 1).as_value<>(),
		Sampler				   = MaterialParameterTypeLayout(MaterialScalarType::Undefined, 0, 0, 0, 0, 1).as_value<>(),
		CombinedImageSampler2D = MaterialParameterTypeLayout(MaterialScalarType::Undefined, 0, 0, 0, 0, 2).as_value<>(),
		Texture2D			   = MaterialParameterTypeLayout(MaterialScalarType::Undefined, 0, 0, 0, 0, 3).as_value<>(),
	};
}// namespace Engine
