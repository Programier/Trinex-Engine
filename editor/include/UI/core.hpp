#pragma once
#include <Core/etl/variant.hpp>
#include <Core/etl/vector.hpp>
#include <Core/math/vector.hpp>
#include <Core/types/name.hpp>

namespace Trinex
{
	class Window;
	class RHITexture;
	class RHISampler;
	class RHIContext;
}// namespace Trinex

namespace Trinex::UI
{
	class Element;

	using Vec2  = Vector2f;
	using Vec3  = Vector3f;
	using Vec4  = Vector4f;
	using Color = Vector4f;

	struct Axis {
		enum Enum : u8
		{
			X,
			Y,
		};

		trinex_enum_struct(Axis);
	};

	struct Ease {
		enum Enum : u8
		{
			Linear = 0,

			InQuad    = 1,
			OutQuad   = 2,
			InOutQuad = 3,

			InCubic    = 4,
			OutCubic   = 5,
			InOutCubic = 6,

			InExpo    = 7,
			OutExpo   = 8,
			InOutExpo = 9,

			OutBack = 10,
		};

		trinex_enum_struct(Ease);
	};

	struct Unit {
		enum Type : u8
		{
			Px,
			Rem,
			Percent,
			Fill,
		};

		Type type = Px;
		f32 value = 0.0f;

		constexpr Unit() = default;
		constexpr Unit(Type type, f32 value = 0.0f) : type(type), value(value) {}
		constexpr Unit(f32 value) : type(Px), value(value) {}
	};

	struct Size {
		Unit width;
		Unit height;

		constexpr Size() = default;
		constexpr Size(Unit width, Unit height) : width(width), height(height) {}
		constexpr Size(f32 width, f32 height) : width(width), height(height) {}
		constexpr Size(const Vec2& size) : width(size.x), height(size.y) {}
	};

	struct Event {
		struct Flags {
			enum Enum : u8
			{
				Undefined = 0,
				Handled   = 1 << 0,
				Bubling   = 1 << 1,
			};

			trinex_bitfield_enum_struct(Flags, u8);
		};

		using enum Flags::Enum;

		Element* sender  = nullptr;
		Element* current = nullptr;
		Flags flags      = Flags::Bubling;

		inline Event& handle()
		{
			flags |= Handled;
			return *this;
		}

		inline Event& unhandle()
		{
			flags &= ~Handled;
			return *this;
		}

		inline Event& stop_propagation()
		{
			flags &= ~Bubling;
			return *this;
		}

		inline Event& resume_propagation()
		{
			flags |= Bubling;
			return *this;
		}

		inline bool handled() const { return flags & Handled; }
		inline bool bubbling() const { return flags & Bubling; }
	};

	namespace Markup
	{
		struct SourceLocation {
			u32 line   = 0;
			u32 column = 0;
		};

		struct LocalizationKey : public Vector<Name> {
			using Vector::Vector;
		};


		struct BindingPath : public Vector<Name> {
			struct Mode {
				enum Enum : u8
				{
					Undefined = 0,
					Read      = 1 << 0,
					Write     = 1 << 1,
					RW        = Read | Write,
				};

				trinex_bitfield_enum_struct(Mode, u8);
			};

			using Vector::Vector;
			Mode mode = Mode::Read;
		};

		struct PropertyPath : public Vector<Name> {
			using Vector::Vector;
		};

		struct Identifier : public String {
			using String::String;
		};

		struct Null {
		};

		using Container = Vector<struct ValueDesc>;
		using Object    = Vector<struct ObjectField>;
		using Value = Variant<Null, bool, i32, f32, String, Unit, LocalizationKey, BindingPath, Identifier, Container, Object>;

		struct ValueDesc {
			Value value;
			SourceLocation location;
		};

		struct ObjectField {
			Name name;
			ValueDesc value;
			SourceLocation location;
		};

	}// namespace Markup
}// namespace Trinex::UI
