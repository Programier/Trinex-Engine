#include "bind_glm.hpp"
#include "bind_glm_functions.hpp"
#include <Core/engine_loading_controllers.hpp>

namespace Engine
{

#define implement_vector_wrapper_bool(name)                                                                                      \
	struct name##Wrapper : public name {                                                                                         \
		name& obj()                                                                                                              \
		{                                                                                                                        \
			return *this;                                                                                                        \
		}                                                                                                                        \
                                                                                                                                 \
		const name& obj() const                                                                                                  \
		{                                                                                                                        \
			return *this;                                                                                                        \
		}                                                                                                                        \
                                                                                                                                 \
		name##Wrapper()                                                                                                          \
		{}                                                                                                                       \
                                                                                                                                 \
		template<typename... Args>                                                                                               \
		name##Wrapper(Args... args) : name(args...)                                                                              \
		{}                                                                                                                       \
                                                                                                                                 \
		name##Wrapper(const name##Wrapper& vec)                                                                                  \
		{                                                                                                                        \
			obj() = vec.obj();                                                                                                   \
		}                                                                                                                        \
                                                                                                                                 \
		bool operator==(const name##Wrapper& wrap) const                                                                         \
		{                                                                                                                        \
			return obj() == wrap.obj();                                                                                          \
		}                                                                                                                        \
                                                                                                                                 \
		name##Wrapper& operator=(const name##Wrapper& new_obj)                                                                   \
		{                                                                                                                        \
			obj() = new_obj.obj();                                                                                               \
			return *this;                                                                                                        \
		}                                                                                                                        \
                                                                                                                                 \
		name##Wrapper& operator+=(const name##Wrapper& new_obj)                                                                  \
		{                                                                                                                        \
			obj() += new_obj.obj();                                                                                              \
			return *this;                                                                                                        \
		}                                                                                                                        \
                                                                                                                                 \
		name##Wrapper& operator-=(const name##Wrapper& new_obj)                                                                  \
		{                                                                                                                        \
			obj() -= new_obj.obj();                                                                                              \
			return *this;                                                                                                        \
		}                                                                                                                        \
                                                                                                                                 \
		name##Wrapper& operator/=(const name##Wrapper& new_obj)                                                                  \
		{                                                                                                                        \
			obj() /= new_obj.obj();                                                                                              \
			return *this;                                                                                                        \
		}                                                                                                                        \
                                                                                                                                 \
		name##Wrapper& operator*=(const name##Wrapper& new_obj)                                                                  \
		{                                                                                                                        \
			obj() *= new_obj.obj();                                                                                              \
			return *this;                                                                                                        \
		}                                                                                                                        \
                                                                                                                                 \
		name##Wrapper& operator+=(name::value_type v)                                                                            \
		{                                                                                                                        \
			obj() += v;                                                                                                          \
			return *this;                                                                                                        \
		}                                                                                                                        \
                                                                                                                                 \
		name##Wrapper& operator-=(name::value_type v)                                                                            \
		{                                                                                                                        \
			obj() -= v;                                                                                                          \
			return *this;                                                                                                        \
		}                                                                                                                        \
                                                                                                                                 \
		name##Wrapper& operator/=(name::value_type v)                                                                            \
		{                                                                                                                        \
			obj() /= v;                                                                                                          \
			return *this;                                                                                                        \
		}                                                                                                                        \
                                                                                                                                 \
		name##Wrapper& operator*=(name::value_type v)                                                                            \
		{                                                                                                                        \
			obj() &= v;                                                                                                          \
			return *this;                                                                                                        \
		}                                                                                                                        \
                                                                                                                                 \
		name##Wrapper operator+(const name##Wrapper& new_obj) const                                                              \
		{                                                                                                                        \
			return obj() + new_obj.obj();                                                                                        \
		}                                                                                                                        \
                                                                                                                                 \
		name##Wrapper operator+(name::value_type v) const                                                                        \
		{                                                                                                                        \
			return obj() + v;                                                                                                    \
		}                                                                                                                        \
                                                                                                                                 \
		name##Wrapper operator-(const name##Wrapper& new_obj) const                                                              \
		{                                                                                                                        \
			return obj() - new_obj.obj();                                                                                        \
		}                                                                                                                        \
                                                                                                                                 \
		name##Wrapper operator-(name::value_type v) const                                                                        \
		{                                                                                                                        \
			return obj() - v;                                                                                                    \
		}                                                                                                                        \
                                                                                                                                 \
		name##Wrapper operator*(const name##Wrapper& new_obj) const                                                              \
		{                                                                                                                        \
			return obj() & new_obj.obj();                                                                                        \
		}                                                                                                                        \
                                                                                                                                 \
		name##Wrapper operator*(name::value_type v) const                                                                        \
		{                                                                                                                        \
			return obj() & v;                                                                                                    \
		}                                                                                                                        \
                                                                                                                                 \
		name##Wrapper operator/(const name##Wrapper& new_obj) const                                                              \
		{                                                                                                                        \
			return obj() / new_obj.obj();                                                                                        \
		}                                                                                                                        \
                                                                                                                                 \
		name##Wrapper operator/(name::value_type v) const                                                                        \
		{                                                                                                                        \
			return obj() / v;                                                                                                    \
		}                                                                                                                        \
                                                                                                                                 \
		name##Wrapper reverse_operator_sub(name::value_type f)                                                                   \
		{                                                                                                                        \
			return f - obj();                                                                                                    \
		}                                                                                                                        \
                                                                                                                                 \
		name##Wrapper reverse_operator_div(name::value_type f)                                                                   \
		{                                                                                                                        \
			return f / obj();                                                                                                    \
		}                                                                                                                        \
                                                                                                                                 \
		name::value_type value_at(uint_t index) const                                                                            \
		{                                                                                                                        \
			return obj()[index];                                                                                                 \
		}                                                                                                                        \
                                                                                                                                 \
		name::value_type& value_at(uint_t index)                                                                                 \
		{                                                                                                                        \
			return obj()[index];                                                                                                 \
		}                                                                                                                        \
	};


	implement_vector_wrapper_bool(BoolVector1D);
	implement_vector_wrapper_bool(BoolVector2D);
	implement_vector_wrapper_bool(BoolVector3D);
	implement_vector_wrapper_bool(BoolVector4D);

	static void bind_functions()
	{
		String vtype = "bool";
		bind_wrapped_functions<BoolVector1DWrapper, BoolVector1D, All ^ Length ^ Cross ^ Normalize ^ Dot>("Engine::BoolVector1D",
		                                                                                                  vtype);
		bind_wrapped_functions<BoolVector2DWrapper, BoolVector2D, All ^ Length ^ Cross ^ Normalize ^ Dot>("Engine::BoolVector2D",
		                                                                                                  vtype);
		bind_wrapped_functions<BoolVector3DWrapper, BoolVector3D, All ^ Length ^ Cross ^ Normalize ^ Dot>("Engine::BoolVector3D",
		                                                                                                  vtype);
		bind_wrapped_functions<BoolVector4DWrapper, BoolVector4D, All ^ Length ^ Cross ^ Normalize ^ Dot>("Engine::BoolVector4D",
		                                                                                                  vtype);
	}

	static void on_init()
	{

		String prop_type       = "bool";
		String class_name      = "Engine::BoolVector";
		const char* const_type = "bool";
		const char* ref_type   = "bool&";
		using RegistryType1    = BoolVector1DWrapper;
		using RegistryType2    = BoolVector2DWrapper;
		using RegistryType3    = BoolVector3DWrapper;
		using RegistryType4    = BoolVector4DWrapper;

		{
			using ConstType = RegistryType1::value_type;
			using RefType   = RegistryType1::value_type&;
			ScriptClassRegistrar registrar =
			        ScriptClassRegistrar::value_class(class_name + "1D", sizeof(RegistryType1), info_of<RegistryType1>());
			bind_glm_behaviours<RegistryType1>(registrar, prop_type);
			bind_vec1_props<RegistryType1>(registrar, prop_type);
			bind_glm_operators<RegistryType1>(registrar, prop_type);
			bind_index_op<RegistryType1, ConstType, RefType>(registrar, const_type, ref_type);
		}
		{
			using ConstType = RegistryType2::value_type;
			using RefType   = RegistryType2::value_type&;
			ScriptClassRegistrar registrar =
			        ScriptClassRegistrar::value_class(class_name + "2D", sizeof(RegistryType2), info_of<RegistryType2>());
			bind_glm_behaviours<RegistryType2>(registrar, prop_type);
			bind_vec2_props<RegistryType2>(registrar, prop_type);
			bind_glm_operators<RegistryType2>(registrar, prop_type);
			bind_index_op<RegistryType2, ConstType, RefType>(registrar, const_type, ref_type);
		}
		{
			using ConstType = RegistryType3::value_type;
			using RefType   = RegistryType3::value_type&;
			ScriptClassRegistrar registrar =
			        ScriptClassRegistrar::value_class(class_name + "3D", sizeof(RegistryType3), info_of<RegistryType3>());
			bind_glm_behaviours<RegistryType3>(registrar, prop_type);
			bind_vec3_props<RegistryType3>(registrar, prop_type);
			bind_glm_operators<RegistryType3>(registrar, prop_type);
			bind_index_op<RegistryType3, ConstType, RefType>(registrar, const_type, ref_type);
		}
		{
			using ConstType = RegistryType3::value_type;
			using RefType   = RegistryType3::value_type&;
			ScriptClassRegistrar registrar =
			        ScriptClassRegistrar::value_class(class_name + "4D", sizeof(RegistryType4), info_of<RegistryType4>());
			bind_glm_behaviours<RegistryType4>(registrar, prop_type);
			bind_vec4_props<RegistryType4>(registrar, prop_type);
			bind_glm_operators<RegistryType4>(registrar, prop_type);
			bind_index_op<RegistryType4, ConstType, RefType>(registrar, const_type, ref_type);
		}
		bind_functions();
	}

	static ReflectionInitializeController controller(on_init, "Engine::BoolVector");
}// namespace Engine
