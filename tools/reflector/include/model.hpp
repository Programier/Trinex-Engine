#pragma once
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace Reflector
{
#define trinex_reflector_type(type, super)                                                                                       \
	using This  = type;                                                                                                          \
	using Super = super;                                                                                                         \
	ObjectKind kind() const override;                                                                                            \
	virtual bool is_a(ObjectKind kind) const override;                                                                           \
	virtual void serialize(Archive& ar) override

#define trinex_implement_reflector_type(type, ...)                                                                               \
	ObjectKind type::kind() const                                                                                                \
	{                                                                                                                            \
		return ObjectKind::type;                                                                                                 \
	}                                                                                                                            \
	bool type::is_a(ObjectKind kind) const                                                                                       \
	{                                                                                                                            \
		if (kind == ObjectKind::type)                                                                                            \
			return true;                                                                                                         \
		return Super::is_a(kind);                                                                                                \
	}                                                                                                                            \
	void type::serialize(Archive& ar)                                                                                            \
	{                                                                                                                            \
		Super::serialize(ar);                                                                                                    \
		ar(__VA_ARGS__);                                                                                                         \
	}

	enum class Access : std::uint8_t
	{
		Global,
		Private,
		Protected,
		Public,
	};

	enum class ObjectKind : std::uint8_t
	{
		Object    = 0,
		Enum      = 1,
		Scope     = 2,
		Namespace = 3,
		Struct    = 4,
		Class     = 5,
		Module    = 6,
		Function  = 7,
		Property  = 8,
	};

	enum class DiagnosticSeverity : std::uint8_t
	{
		Error,
		Warning,
	};

	struct Diagnostic {
		DiagnosticSeverity severity = DiagnosticSeverity::Error;
		std::string message;
		std::size_t line   = 0;
		std::size_t column = 0;
	};

	class Archive;
	class Object;
	class Enum;
	class Scope;
	class Namespace;
	class Struct;
	class Class;
	class Module;
	class Function;
	class Property;

	struct Metadata {
		std::string name;
		std::string value;

		template<typename Ar>
		void serialize(Ar& ar)
		{
			ar(name, value);
		}
	};

	struct TypeInfo {
		struct TemplateArgument;

		static constexpr inline std::uint8_t Const           = 1 << 0;
		static constexpr inline std::uint8_t Volatile        = 1 << 1;
		static constexpr inline std::uint8_t Reference       = 1 << 2;
		static constexpr inline std::uint8_t RValueRef       = 1 << 3;
		static constexpr inline std::uint8_t Template        = 1 << 4;
		static constexpr inline std::uint8_t Pointer         = 1 << 5;
		static constexpr inline std::uint8_t PointerConst    = 1 << 6;
		static constexpr inline std::uint8_t PointerVolatile = 1 << 7;

		// Pointer represents exactly one indirection (including a decayed array).
		// Const/Volatile qualify the pointee; PointerConst/PointerVolatile qualify the pointer.

		std::string name;
		std::vector<TemplateArgument> templates;
		std::uint8_t flags = 0;

		template<typename Ar>
		void serialize(Ar& ar)
		{
			ar(name, templates, flags);
		}
	};

	struct TypeInfo::TemplateArgument {
		static constexpr inline std::uint8_t Type          = 1 << 0;
		static constexpr inline std::uint8_t Value         = 1 << 1;
		static constexpr inline std::uint8_t Template      = 1 << 2;
		static constexpr inline std::uint8_t PackExpansion = 1 << 3;

		// Exactly one of Type/Value/Template; PackExpansion adds ... after the argument.
		TypeInfo type;    // Type argument, including nested template arguments and qualifiers.
		std::string value;// C++ expression for Value, qualified template name for Template.
		std::uint8_t flags = Type;

		template<typename Ar>
		void serialize(Ar& ar)
		{
			ar(type, value, flags);
		}
	};

	class Object
	{
	public:
		// Non-owning link to the parent scope.
		Object* owner    = nullptr;
		std::string name = "";
		Access access    = Access::Global;
		std::vector<Metadata> metadata;

	public:
		static Object* create(ObjectKind kind);
		virtual Object* find(std::string_view name, ObjectKind kind);
		virtual ObjectKind kind() const;
		virtual bool is_a(ObjectKind kind) const;
		virtual void serialize(Archive& ar);
		virtual ~Object();
	};

	class Enum : public Object
	{
	public:
		struct Value {
			std::string name;
			std::int64_t value;
			std::vector<Metadata> metadata;

			template<typename Ar>
			void serialize(Ar& ar)
			{
				ar(name, value, metadata);
			}
		};

		std::vector<Value> values = {};

	public:
		trinex_reflector_type(Enum, Object);
	};

	class Scope : public Object
	{
	public:
		std::vector<Object*> objects;

	public:
		trinex_reflector_type(Scope, Object);
		Object* find(std::string_view name, ObjectKind kind) override;
		~Scope();
	};

	class Struct : public Scope
	{
	public:
		static constexpr inline std::uint8_t Final = 1 << 0;

		struct Base {
			static constexpr inline std::uint8_t Virtual = 1 << 0;

			TypeInfo type;
			Access access      = Access::Public;
			std::uint8_t flags = 0;
		};

		std::vector<Base> bases;
		std::uint8_t flags = 0;

	public:
		trinex_reflector_type(Struct, Scope);
	};

	class Class : public Struct
	{
	public:
		trinex_reflector_type(Class, Struct);
	};

	class Namespace : public Scope
	{
	public:
		trinex_reflector_type(Namespace, Scope);
	};

	class Module : public Scope
	{
	public:
		trinex_reflector_type(Module, Scope);
	};

	class Function : public Object
	{
	public:
		static constexpr inline std::uint32_t Static      = 1 << 0;
		static constexpr inline std::uint32_t Const       = 1 << 1;
		static constexpr inline std::uint32_t Volatile    = 1 << 2;
		static constexpr inline std::uint32_t Constexpr   = 1 << 3;
		static constexpr inline std::uint32_t Virtual     = 1 << 4;
		static constexpr inline std::uint32_t Inline      = 1 << 5;
		static constexpr inline std::uint32_t Noexcept    = 1 << 6;
		static constexpr inline std::uint32_t Override    = 1 << 7;
		static constexpr inline std::uint32_t Final       = 1 << 8;
		static constexpr inline std::uint32_t PureVirtual = 1 << 9;
		static constexpr inline std::uint32_t Constructor = 1 << 10;
		static constexpr inline std::uint32_t Destructor  = 1 << 11;
		static constexpr inline std::uint32_t Operator    = 1 << 12;
		static constexpr inline std::uint32_t Template    = 1 << 13;
		static constexpr inline std::uint32_t Variadic    = 1 << 14;
		static constexpr inline std::uint32_t Reference   = 1 << 15;
		static constexpr inline std::uint32_t RValueRef   = 1 << 16;

		struct Argument {
			TypeInfo type;
			std::string name;
			std::string value;

			template<typename Ar>
			void serialize(Ar& ar)
			{
				ar(type, name, value);
			}
		};

		TypeInfo type;
		std::vector<Argument> args;
		std::uint32_t flags = 0;

	public:
		trinex_reflector_type(Function, Object);
	};

	class Property : public Object
	{
	public:
		static constexpr inline std::uint8_t Static    = 1 << 0;
		static constexpr inline std::uint8_t Constexpr = 1 << 1;
		static constexpr inline std::uint8_t Mutable   = 1 << 2;
		static constexpr inline std::uint8_t Inline    = 1 << 3;
		static constexpr inline std::uint8_t Bitfield  = 1 << 4;

		TypeInfo type;
		std::string value;
		std::uint8_t flags = 0;

	public:
		trinex_reflector_type(Property, Object);
	};

	void serialize(Archive& ar, Object*& object);
}// namespace Reflector
