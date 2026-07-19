#pragma once

#include <Core/etl/templates.hpp>
#include <ScriptEngine/enums.hpp>
#include <ScriptEngine/script_function.hpp>
#include <ScriptEngine/script_type_info.hpp>

class asIScriptEngine;
class asIScriptGeneric;
struct asSFuncPtr;

namespace Trinex
{
	class ScriptEngine;

	namespace Refl
	{
		class Class;
	}

	namespace ScriptBinding
	{
		class ENGINE_EXPORT FunctionPointer
		{
		private:
			using FunctionPtr = void (*)();
			using GenericPtr  = void (*)(asIScriptGeneric*);
			using MethodPtr   = void (ScriptFunction::*)();

			struct Private {
			};

		private:
			alignas(8) u8 m_storage[40];

		private:
			FunctionPointer(FunctionPtr ptr, Private);
			FunctionPointer(MethodPtr ptr, Private);

		public:
			FunctionPointer() = default;
			trinex_default_copyable(FunctionPointer);
			trinex_default_moveable(FunctionPointer);

			template<typename Ret, typename... Args>
			FunctionPointer(Ret (*func)(Args...)) : FunctionPointer(reinterpret_cast<FunctionPtr>(func), Private())
			{}

			FunctionPointer(GenericPtr ptr);

			template<typename Ret, typename Instance, typename... Args>
			FunctionPointer(Ret (Instance::*func)(Args...)) : FunctionPointer(reinterpret_cast<MethodPtr>(func), Private())
			{}

			template<typename Ret, typename Instance, typename... Args>
			FunctionPointer(Ret (Instance::*func)(Args...) const) : FunctionPointer(reinterpret_cast<MethodPtr>(func), Private())
			{}

			inline asSFuncPtr& reference() { return *reinterpret_cast<asSFuncPtr*>(&m_storage[0]); }
			inline const asSFuncPtr& reference() const { return *reinterpret_cast<const asSFuncPtr*>(&m_storage[0]); }

			inline operator asSFuncPtr&() { return reference(); }
			inline operator const asSFuncPtr&() const { return reference(); }
		};


		struct ENGINE_EXPORT ObjectTypeOptions {
			usize size             = 0;
			ScriptClassFlags flags = 0;
		};

		struct ENGINE_EXPORT Helpers {
			template<typename T, typename... Args>
			static void constructor(T* memory, Args... args)
			{
				new (memory) T(args...);
			}

			template<typename T>
			static void destructor(T* memory)
			{
				memory->~T();
			}

			template<typename T, typename B>
			static T& assign(T* self, B other)
			{
				(*self) = other;
				return *self;
			}
		};

		ENGINE_EXPORT ObjectTypeOptions value_type(usize size, ScriptClassFlags flags = 0);
		ENGINE_EXPORT ObjectTypeOptions reference_type(usize size = 0, ScriptClassFlags flags = ScriptClassFlags::NoCount);

		template<typename T>
		ObjectTypeOptions value_type(ScriptClassFlags flags = 0)
		{
			flags |= ScriptClassFlags::Value;

			if constexpr (std::is_floating_point_v<T>)
			{
				flags |= ScriptClassFlags::AppFloat;
			}
			else if constexpr (std::is_integral_v<T> || std::is_pointer_v<T> || std::is_enum_v<T>)
			{
				flags |= ScriptClassFlags::AppPrimitive;
			}
			else if constexpr (std::is_array_v<T>)
			{
				flags |= ScriptClassFlags::AppArray;
			}
			else if constexpr (std::is_class_v<T>)
			{
				flags |= ScriptClassFlags::AppClass;

				if constexpr (std::is_default_constructible_v<T> && !std::is_trivially_default_constructible_v<T>)
					flags |= ScriptClassFlags::AppClassConstructor;

				if constexpr (std::is_copy_assignable_v<T> && !std::is_trivially_copy_assignable_v<T>)
					flags |= ScriptClassFlags::AppClassAssignment;

				if constexpr (std::is_copy_constructible_v<T> && !std::is_trivially_copy_constructible_v<T>)
					flags |= ScriptClassFlags::AppClassCopyCtor;

				if constexpr (std::is_destructible_v<T> && !std::is_trivially_destructible_v<T>)
					flags |= ScriptClassFlags::AppClassDestructor;
			}

			return value_type(sizeof(T), flags);
		}

		template<typename T>
		ObjectTypeOptions reference_type(ScriptClassFlags flags = ScriptClassFlags::NoCount)
		{
			return reference_type(sizeof(T), flags);
		}

		class ENGINE_EXPORT Class
		{
			String m_class;
			String m_class_base;
			String m_namespace;

			Class(const StringView& name);

		public:
			static Class create(const StringView& name, const ObjectTypeOptions& options = {});
			static Class existing(const StringView& name);
			static Class existing(Refl::Class* class_instance);
			static Class reflected(Refl::Class* class_instance,
			                       ScriptClassFlags flags = ScriptClassFlags::AppNativeInheritance | ScriptClassFlags::NoCount);

			inline const String& name() const { return m_class; }
			inline const String& base_name() const { return m_class_base; }
			inline const String& namespace_name() const { return m_namespace; }

			ScriptTypeInfo type_info() const;
			i32 type_id() const;

			Class& behaviour(ScriptClassBehave behaviour, const char* decl, const FunctionPointer& func,
			                 ScriptCallConv conv = ScriptCallConv::Auto, void* auxiliary = nullptr);

			ScriptFunction method(const char* decl, const FunctionPointer& func, ScriptCallConv conv = ScriptCallConv::Auto,
			                      void* auxiliary = nullptr);

			Class& static_function(const char* decl, const FunctionPointer& func, ScriptCallConv conv = ScriptCallConv::Auto,
			                       void* auxiliary = nullptr);

			Class& property(const char* decl, usize offset);
			Class& static_property(const char* decl, void* property);

			Class& constructor(const char* decl, const FunctionPointer& func, ScriptCallConv conv = ScriptCallConv::Auto,
			                   void* auxiliary = nullptr);
			Class& destructor(const char* decl, const FunctionPointer& func, ScriptCallConv conv = ScriptCallConv::Auto,
			                  void* auxiliary = nullptr);
			Class& factory(const char* decl, const FunctionPointer& func, ScriptCallConv conv = ScriptCallConv::CDecl,
			               void* auxiliary = nullptr);
			Class& addref(const char* decl, const FunctionPointer& func, ScriptCallConv conv = ScriptCallConv::Auto,
			              void* auxiliary = nullptr);
			Class& release(const char* decl, const FunctionPointer& func, ScriptCallConv conv = ScriptCallConv::Auto,
			               void* auxiliary = nullptr);
			Class& template_callback(const char* decl, const FunctionPointer& func, ScriptCallConv conv = ScriptCallConv::CDecl,
			                         void* auxiliary = nullptr);
			Class& funcdef(const String& decl);


			template<typename T, typename C>
			Class& property(const char* decl, T C::* property)
			{
				return this->property(decl, offset_of(property));
			}

			template<typename T, typename... Args>
			Class& constructor(const char* decl = "void f()")
			{
				return constructor(decl, Helpers::constructor<T, Args...>, ScriptCallConv::CDeclObjFirst);
			}

			template<typename T>
			Class& destructor(const char* decl = "void f()")
			{
				return destructor(decl, Helpers::destructor<T>, ScriptCallConv::CDeclObjFirst);
			}
		};

		class ENGINE_EXPORT Enum
		{
			String m_base;
			String m_namespace;

		public:
			Enum(const StringView& namespace_name, const StringView& base_name, bool init = true);
			explicit Enum(const StringView& full_name, bool init = true);

			inline const String& base_name() const { return m_base; }
			inline const String& namespace_name() const { return m_namespace; }

			Enum& value(const char* name, i64 value);
			ScriptTypeInfo type_info() const;
			i32 type_id() const;

			template<typename T>
			Enum& value(const char* name, T enum_value)
			{
				return value(name, static_cast<i64>(enum_value));
			}
		};

		class ENGINE_EXPORT Namespace
		{
		private:
			String m_namespace;

		public:
			explicit Namespace(const StringView& namespace_name = "");

			const String& name() const;
			Namespace nested(const StringView& suffix) const;

			Namespace& property(const char* decl, void* property);
			Namespace& func_def(const char* decl);
			Namespace& type_def(const char* new_type_name, const char* type);

			Class object_type(const StringView& name, const ObjectTypeOptions& options = {}) const;
			Class existing_class(const StringView& name) const;
			Enum enum_type(const StringView& name, bool init = true) const;

			Namespace& function(const char* decl, const FunctionPointer& func, ScriptCallConv conv, void* auxiliary);

			template<typename T>
			Class value_class(const StringView& name, ScriptClassFlags flags = 0) const
			{
				return object_type(name, value_type_options<T>(flags));
			}

			template<typename T>
			Class reference_class(const StringView& name, ScriptClassFlags flags = ScriptClassFlags::NoCount) const
			{
				return object_type(name, reference_type_options<T>(flags));
			}
		};

		ENGINE_EXPORT Namespace global();
		ENGINE_EXPORT Namespace in(const StringView& namespace_name);
	}// namespace ScriptBinding
}// namespace Trinex
