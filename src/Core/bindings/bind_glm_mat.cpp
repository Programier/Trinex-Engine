#include "bind_glm.hpp"
#include <Core/engine_loading_controllers.hpp>
#include <sstream>


namespace Engine
{
#define implement_matrix_wrapper(name)                                                                                           \
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
			obj() *= v;                                                                                                          \
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
			return obj() * new_obj.obj();                                                                                        \
		}                                                                                                                        \
                                                                                                                                 \
		name##Wrapper operator*(name::value_type v) const                                                                        \
		{                                                                                                                        \
			return obj() * v;                                                                                                    \
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
		String as_string() const                                                                                                 \
		{                                                                                                                        \
			std::stringstream s;                                                                                                 \
			s << obj();                                                                                                          \
			return s.str();                                                                                                      \
		}                                                                                                                        \
                                                                                                                                 \
		const name::col_type& value_at(uint_t index) const                                                                       \
		{                                                                                                                        \
			return obj()[index];                                                                                                 \
		}                                                                                                                        \
                                                                                                                                 \
		name::col_type& value_at(uint_t index)                                                                                   \
		{                                                                                                                        \
			return obj()[index];                                                                                                 \
		}                                                                                                                        \
	};


	implement_matrix_wrapper(Matrix4f);
	implement_matrix_wrapper(Matrix3f);
	implement_matrix_wrapper(Matrix2f);


	static void on_init()
	{
		ReflectionInitializeController().require("Engine::Vector");
		String prop_type  = "float";
		String class_name = "Engine::Matrix";

		{
			using ConstType = const Matrix4f::col_type&;
			using RefType   = Matrix4f::col_type&;
			ScriptClassRegistrar registrar =
			        ScriptClassRegistrar::value_class(class_name + "4f", sizeof(Matrix4fWrapper), info_of<Matrix4fWrapper>());
			bind_glm_behaviours<Matrix4fWrapper>(registrar, prop_type);
			bind_glm_operators<Matrix4fWrapper>(registrar, prop_type);
			bind_index_op<Matrix4fWrapper, ConstType, RefType>(registrar, "const Engine::Vector4D&", "Engine::Vector4D&");
		}
		{
			using ConstType = const Matrix3f::col_type&;
			using RefType   = Matrix3f::col_type&;
			ScriptClassRegistrar registrar =
			        ScriptClassRegistrar::value_class(class_name + "3f", sizeof(Matrix3fWrapper), info_of<Matrix3fWrapper>());
			bind_glm_behaviours<Matrix3fWrapper>(registrar, prop_type);
			bind_glm_operators<Matrix3fWrapper>(registrar, prop_type);
			bind_index_op<Matrix3fWrapper, ConstType, RefType>(registrar, "const Engine::Vector3D&", "Engine::Vector3D&");
		}
		{
			using ConstType = const Matrix2f::col_type&;
			using RefType   = Matrix2f::col_type&;
			ScriptClassRegistrar registrar =
			        ScriptClassRegistrar::value_class(class_name + "2f", sizeof(Matrix2fWrapper), info_of<Matrix2fWrapper>());
			bind_glm_behaviours<Matrix2fWrapper>(registrar, prop_type);
			bind_glm_operators<Matrix2fWrapper>(registrar, prop_type);
			bind_index_op<Matrix2fWrapper, ConstType, RefType>(registrar, "const Engine::Vector2D&", "Engine::Vector2D&");
		}
	}

	static ReflectionInitializeController controller(on_init, "Engine::Matrix");
}// namespace Engine
