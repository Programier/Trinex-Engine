#pragma once
#include <Core/engine_types.hpp>
#include <Core/enums.hpp>

namespace Engine
{
	class ScriptTypeInfo;

	class ENGINE_EXPORT ScriptVariableBase
	{
	protected:
		mutable ScriptTypeModifiers m_modifiers;

		union
		{
			mutable void* m_address;
			bool m_bool_value;
			int8_t m_int8_value;
			int16_t m_int16_value;
			int32_t m_int32_value;
			int64_t m_int64_value;
			uint8_t m_uint8_value;
			uint16_t m_uint16_value;
			uint32_t m_uint32_value;
			uint64_t m_uint64_value;
			float m_float_value;
			double m_double_value;
		};
		mutable int_t m_type_id;

		bool check_type(int_t mask) const;
		virtual const ScriptVariableBase& add_ref() const;

	public:
		ScriptVariableBase();
		ScriptVariableBase(const ScriptTypeInfo& info, bool is_uninitialized = false);
		ScriptVariableBase(void* src_address, const ScriptTypeInfo& info, bool handle_is_object = false,
		                   const ScriptTypeModifiers& modifiers = {});
		copy_constructors_hpp(ScriptVariableBase);

		bool operator==(const ScriptVariableBase& other) const;
		bool operator!=(const ScriptVariableBase& other) const;

		virtual const ScriptVariableBase& release() const;

		bool assign(const ScriptVariableBase& other);
		bool assign(void* address, bool handle_is_object = false);
		bool assign_to(ScriptVariableBase& other) const;
		bool assign_to(void* address) const;

		bool create(const ScriptTypeInfo& info, bool is_uninitialized = false);
		bool create(void* src_address, const ScriptTypeInfo& info, bool handle_is_object = false,
		            const ScriptTypeModifiers& modifiers = {});

		bool is_valid() const;
		bool is_object(bool handle_is_object = false) const;
		bool is_handle() const;
		bool is_const() const;
		bool is_in_ref() const;
		bool is_out_ref() const;
		bool is_inout_ref() const;
		void* address() const;
		int_t type_id() const;
		virtual ScriptTypeInfo type_info() const;

		template<typename T>
		T* address_as() const
		{
			return reinterpret_cast<T*>(address());
		}

		~ScriptVariableBase();
	};

	class ENGINE_EXPORT ScriptVariable : public ScriptVariableBase
	{
	public:
		using ScriptVariableBase::ScriptVariableBase;

		ScriptVariable(int_t type_id);
		ScriptVariable(const char* declaration);
		ScriptVariable(const char* declaration, const char* module);
		ScriptVariable(void* address, int_t type_id, bool handle_is_object = false, const ScriptTypeModifiers& modifiers = {});
		ScriptVariable(void* address, const char* declaration, bool handle_is_object = false,
		               const ScriptTypeModifiers& modifiers = {});
		ScriptVariable(void* address, const char* declaration, const char* module, bool handle_is_object = false,
		               const ScriptTypeModifiers& modifiers = {});

		ScriptVariable(const ScriptVariable& object);
		ScriptVariable(ScriptVariable&& object);
		ScriptVariable& operator=(ScriptVariable&&);
		ScriptVariable& operator=(const ScriptVariable&);

		using ScriptVariableBase::create;
		bool create(const ScriptVariableBase& other);
		bool create(int_t type_id, bool is_uninitialized = false);
		bool create(const char* type_declaration, bool is_uninitialized = false);
		bool create(const char* type_declaration, const char* module, bool is_uninitialized = false);
		bool create(void* src_address, int_t type_id, bool handle_is_object = false, const ScriptTypeModifiers& modifiers = {});
		bool create(void* src_address, const char* type_declaration, bool handle_is_object = false,
		            const ScriptTypeModifiers& modifiers = {});
		bool create(void* src_address, const char* type_declaration, const char* module, bool handle_is_object = false,
		            const ScriptTypeModifiers& modifiers = {});

		bool is_bool() const;
		bool is_int8() const;
		bool is_int16() const;
		bool is_int32() const;
		bool is_int64() const;
		bool is_uint8() const;
		bool is_uint16() const;
		bool is_uint32() const;
		bool is_uint64() const;
		bool is_float() const;
		bool is_double() const;

		bool bool_value() const;
		int8_t int8_value() const;
		int16_t int16_value() const;
		int32_t int32_value() const;
		int64_t int64_value() const;
		uint8_t uint8_value() const;
		uint16_t uint16_value() const;
		uint32_t uint32_value() const;
		uint64_t uint64_value() const;
		float float_value() const;
		double double_value() const;
	};
}// namespace Engine
