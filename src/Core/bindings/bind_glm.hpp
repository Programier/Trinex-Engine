#pragma once
#include <Core/engine_types.hpp>
#include <Core/etl/templates.hpp>
#include <Core/etl/type_traits.hpp>
#include <Core/etl/vector.hpp>
#include <Core/string_functions.hpp>

//#include <Core/math/vector.hpp>
#include <Core/math/matrix.hpp>

#include <ScriptEngine/registrar.hpp>
#include <ScriptEngine/script_engine.hpp>
#include <fmt/ranges.h>
#include <glm/ext/matrix_integer.hpp>
#include <glm/gtx/string_cast.hpp>

namespace Engine::Bindings::GLM
{
	using glm_element_types = TypesList<bool, int32_t, uint32_t, float>;

	template<typename T>
	constexpr const char* type_names = nullptr;

#define trinex_bindings_typename(type, name)                                                                                     \
	template<>                                                                                                                   \
	constexpr const char* type_names<type> = #name

#define trinex_bindings_engine_typename(type)                                                                                    \
	template<>                                                                                                                   \
	constexpr const char* type_names<type> = "Engine::" #type

	trinex_bindings_typename(void, void);
	trinex_bindings_typename(bool_t, bool);
	trinex_bindings_typename(int32_t, int32);
	trinex_bindings_typename(uint32_t, uint32);
	trinex_bindings_typename(float_t, float);
	trinex_bindings_engine_typename(Quaternion);
	trinex_bindings_engine_typename(Vector1f);
	trinex_bindings_engine_typename(Vector2f);
	trinex_bindings_engine_typename(Vector3f);
	trinex_bindings_engine_typename(Vector4f);
	trinex_bindings_engine_typename(Vector1b);
	trinex_bindings_engine_typename(Vector2b);
	trinex_bindings_engine_typename(Vector3b);
	trinex_bindings_engine_typename(Vector4b);
	trinex_bindings_engine_typename(Vector1i);
	trinex_bindings_engine_typename(Vector2i);
	trinex_bindings_engine_typename(Vector3i);
	trinex_bindings_engine_typename(Vector4i);
	trinex_bindings_engine_typename(Vector1u);
	trinex_bindings_engine_typename(Vector2u);
	trinex_bindings_engine_typename(Vector3u);
	trinex_bindings_engine_typename(Vector4u);
	trinex_bindings_engine_typename(Matrix4f);
	trinex_bindings_engine_typename(Matrix3f);
	trinex_bindings_engine_typename(Matrix2f);
	trinex_bindings_engine_typename(Matrix4i);
	trinex_bindings_engine_typename(Matrix3i);
	trinex_bindings_engine_typename(Matrix2i);
	trinex_bindings_engine_typename(Matrix4u);
	trinex_bindings_engine_typename(Matrix3u);
	trinex_bindings_engine_typename(Matrix2u);
	trinex_bindings_engine_typename(Matrix4b);
	trinex_bindings_engine_typename(Matrix3b);
	trinex_bindings_engine_typename(Matrix2b);

#undef trinex_bindings_typename_e
#undef trinex_bindings_typename

	template<typename T>
	static consteval const char* typename_of()
	{
		constexpr const char* name = type_names<std::decay_t<std::remove_pointer_t<T>>>;
		static_assert(name);
		return name;
	}

	template<typename T>
	struct is_vector : std::false_type {
	};

	template<typename T>
	struct is_matrix : std::false_type {
	};

	template<typename T>
	struct is_quaternion : std::false_type {
	};

	template<glm::length_t L, typename T, glm::qualifier Q>
	struct is_vector<glm::vec<L, T, Q>> : std::true_type {
	};

	template<glm::length_t C, glm::length_t R, typename T, glm::qualifier Q>
	struct is_matrix<glm::mat<C, R, T, Q>> : std::true_type {
	};

	template<>
	struct is_quaternion<Quaternion> : std::true_type {
	};

	template<typename T>
	constexpr inline bool is_vector_v = is_vector<T>::value;

	template<typename T>
	constexpr inline bool is_matrix_v = is_matrix<T>::value;

	template<typename T>
	constexpr inline bool is_quaternion_v = is_quaternion<T>::value;

	template<typename T, size_t x = 1, size_t y = 1>
	struct up_cast {
	};

	template<glm::length_t L, typename T, glm::qualifier Q, size_t x, size_t y>
	struct up_cast<glm::vec<L, T, Q>, x, y> {
		using Type = glm::vec<L - x, T, Q>;
	};

	template<glm::length_t C, glm::length_t R, typename T, glm::qualifier Q, size_t x, size_t y>
	struct up_cast<glm::mat<C, R, T, Q>, x, y> {
		using Type = glm::mat<C - x, R - y, T, Q>;
	};

	template<typename T, typename E>
	struct redirect {
	};

	template<typename E, glm::length_t L, typename T, glm::qualifier Q>
	struct redirect<glm::vec<L, T, Q>, E> {
		using Type = glm::vec<L, E, Q>;
	};

	template<typename E, glm::length_t C, glm::length_t R, typename T, glm::qualifier Q>
	struct redirect<glm::mat<C, R, T, Q>, E> {
		using Type = glm::mat<C, R, E, Q>;
	};

	namespace Detectors
	{
		template<typename Type, typename T>
		using bind_generic_detector = decltype(T::template bind_generic<Type>(static_cast<ScriptClassRegistrar*>(nullptr)));

		template<typename Type, typename T>
		using bind_detector = decltype(T::template bind<Type>(static_cast<ScriptClassRegistrar*>(nullptr)));
	};// namespace Detectors

	template<typename T>
	static ScriptClassRegistrar create_registrar()
	{
		ScriptClassRegistrar::ValueInfo info = ScriptClassRegistrar::ValueInfo::from<T>();
		info.is_class                        = true;
		info.pod                             = true;
		info.more_constructors               = true;

		using Element = typename T::value_type;

		if constexpr (std::is_floating_point_v<Element>)
		{
			info.all_floats = true;

			if constexpr (alignof(T) == 8)
			{
				info.align8 = true;
			}
		}
		else
		{
			info.all_ints = true;
		}

		return ScriptClassRegistrar::value_class(typename_of<T>(), sizeof(T), info);
	}

	template<typename T>
	static ScriptClassRegistrar exiting_class()
	{
		return ScriptClassRegistrar::existing_class(String(typename_of<T>()));
	}

	template<typename T, typename BinderType>
	static void bind_glm_type()
	{
		ScriptClassRegistrar reg = exiting_class<T>();

		if constexpr (is_detected_v<Detectors::bind_generic_detector, T, BinderType>)
		{
			BinderType::template bind_generic<T>(&reg);
		}

		if constexpr (is_detected_v<Detectors::bind_detector, T, BinderType>)
		{
			BinderType::template bind<T>(&reg);
		}
	}

	template<typename T>
	static String typename_with_modifiers()
	{
		constexpr const char* const_modifier = std::is_const_v<T> ? "const " : "";
		constexpr const char* ref_modifier   = std::is_reference_v<T> || std::is_pointer_v<T> ? "&" : "";
		constexpr const char* name           = type_names<std::decay_t<std::remove_pointer_t<T>>>;
		static_assert(name);
		return fmt::format("{}{}{}", const_modifier, name, ref_modifier);
	}

	template<typename Ret, typename... Args>
	static String function_signature(const char* name, bool is_const = false, bool is_final = false)
	{
		const char* modifiers = "";

		if (is_const && is_final)
		{
			modifiers = " const final";
		}
		else if (is_const)
		{
			modifiers = " const";
		}
		else if (is_final)
		{
			modifiers = " final";
		}

		return Strings::format("{} {}({}){}", typename_with_modifiers<Ret>(), name,
							   fmt::join({typename_with_modifiers<Args>()...}, ", "), modifiers);
	}

	template<typename Type, typename... Args>
	static void bind_constructor(ScriptClassRegistrar* reg)
	{
		String decl = Strings::format("void f({})", fmt::join({typename_with_modifiers<Args>()...}, ", "));
		reg->behave(ScriptClassBehave::Construct, decl.c_str(), reg->constructor<Type, Args...>, ScriptCallConv::CDeclObjFirst);
	}

	template<typename T, typename Ret, typename... Args>
	static void bind_func(ScriptClassRegistrar* reg, const char* name, Ret (*func)(T& self, Args... args))
	{
		String decl = function_signature<Ret, Args...>(name, std::is_const_v<T>, true);
		reg->method(decl.c_str(), func, ScriptCallConv::CDeclObjFirst);
	}

	template<typename Ret, typename... Args>
	static void bind_func(const char* name, Ret (*func)(Args... args))
	{
		String decl = function_signature<Ret, Args...>(name);
		ScriptEngine::register_function(decl.c_str(), func);
	}

	static void* upcast_function(void* address)
	{
		return address;
	}

	template<typename T>
	static void register_upcast(ScriptClassRegistrar* reg)
	{
		constexpr const char* name = type_names<T>;
		static_assert(name);

		String decl = Strings::format("{}& opImplConv() final", name);
		reg->method(decl.c_str(), upcast_function, ScriptCallConv::CDeclObjFirst);
		decl = Strings::format("const {}& opImplConv() const final", name);
		reg->method(decl.c_str(), upcast_function, ScriptCallConv::CDeclObjFirst);
	}

	template<typename T>
	static String make_property(const StringView& prop)
	{
		return Strings::format("{} {}", typename_of<typename T::value_type>(), prop);
	}

	template<typename T>
	static int32_t length_of()
	{
		return static_cast<int32_t>(T::length());
	}

	// OPERATOR BINDINGS

	struct BaseOperators {
		template<typename T>
		static bool opEquals(const T& self, const T& other)
		{
			return self == other;
		}

		template<typename T>
		static decltype(auto) opIndex(T& self, uint32_t index)
		{
			return self[index];
		}

		template<typename T>
		static decltype(auto) opIndexConst(const T& self, uint32_t index)
		{
			return self[index];
		}

		template<typename T>
		static void bind_ops(ScriptClassRegistrar* reg)
		{
			bind_func(reg, "opEquals", opEquals<T>);
			bind_func(reg, "opIndex", opIndex<T>);
			bind_func(reg, "opIndex", opIndexConst<T>);
		}
	};

	template<typename Value>
	struct Operators : BaseOperators {
	};

	struct BooleanOperators : BaseOperators {
		template<typename T, typename Arg>
		static T& opAndAssign(T& self, Arg other)
		{
			return self &= other;
		}

		template<typename T, typename Arg>
		static T& opOrAssign(T& self, Arg other)
		{
			return self |= other;
		}

		template<typename T, typename Arg>
		static T& opXorAssign(T& self, Arg other)
		{
			return self ^= other;
		}

		template<typename T, typename Arg>
		static T opAnd(T& self, Arg other)
		{
			return self & other;
		}

		template<typename T, typename Arg>
		static T opOr(T& self, Arg other)
		{
			return self | other;
		}

		template<typename T, typename Arg>
		static T opXor(T& self, Arg other)
		{
			return self ^ other;
		}

		template<typename T>
		static void bind_ops(ScriptClassRegistrar* reg, bool execute_base = true)
		{
			if (execute_base)
				BaseOperators::bind_ops<T>(reg);

			if constexpr (is_vector_v<T>)
			{
				bind_func(reg, "opAndAssign", opAndAssign<T, const T&>);
				bind_func(reg, "opOrAssign", opOrAssign<T, const T&>);
				bind_func(reg, "opXorAssign", opXorAssign<T, const T&>);
				bind_func(reg, "opAnd", opAnd<T, const T&>);
				bind_func(reg, "opOr", opOr<T, const T&>);
				bind_func(reg, "opXor", opXor<T, const T&>);

				using Value = typename T::value_type;
				bind_func(reg, "opAndAssign", opAndAssign<T, Value>);
				bind_func(reg, "opOrAssign", opOrAssign<T, Value>);
				bind_func(reg, "opXorAssign", opXorAssign<T, Value>);
				bind_func(reg, "opAnd", opAnd<T, Value>);
				bind_func(reg, "opOr", opOr<T, Value>);
				bind_func(reg, "opXor", opXor<T, Value>);
			}
		}
	};

	struct ScalarOperators : BaseOperators {
		template<typename T>
		static T opNeg(T& self)
		{
			return -self;
		}

		template<typename T>
		static T& opPreInc(T& self)
		{
			return ++self;
		}

		template<typename T>
		static T& opPreDec(T& self)
		{
			return --self;
		}

		template<typename T>
		static T opPostInc(T& self)
		{
			return self++;
		}

		template<typename T>
		static T opPostDec(T& self)
		{
			return self--;
		}

		template<typename T, typename Arg>
		static T& opAddAssign(T& self, Arg other)
		{
			return self += other;
		}

		template<typename T, typename Arg>
		static T& opSubAssign(T& self, Arg other)
		{
			return self -= other;
		}

		template<typename T, typename Arg>
		static T& opMulAssign(T& self, Arg other)
		{
			return self *= other;
		}

		template<typename T, typename Arg>
		static T& opDivAssign(T& self, Arg other)
		{
			return self /= other;
		}

		template<typename T, typename Arg>
		static decltype(auto) opAdd(const T& self, Arg other)
		{
			return self + other;
		}

		template<typename T, typename Arg>
		static decltype(auto) opSub(const T& self, Arg other)
		{
			return self - other;
		}

		template<typename T, typename Arg>
		static decltype(auto) opMul(const T& self, Arg other)
		{
			return self * other;
		}

		template<typename T, typename Arg>
		static decltype(auto) opDiv(const T& self, Arg other)
		{
			return self / other;
		}

		template<typename T>
		static void bind_ops(ScriptClassRegistrar* reg)
		{
			BaseOperators::bind_ops<T>(reg);

			using Value = typename T::value_type;

			bind_func(reg, "opNeg", opNeg<T>);

			if constexpr (!is_quaternion_v<T>)
			{
				bind_func(reg, "opPreInc", opPreInc<T>);
				bind_func(reg, "opPreDec", opPreDec<T>);
				bind_func(reg, "opPostInc", opPostInc<T>);
				bind_func(reg, "opPostDec", opPostDec<T>);
			}

			bind_func(reg, "opAddAssign", opAddAssign<T, const T&>);
			bind_func(reg, "opSubAssign", opSubAssign<T, const T&>);
			bind_func(reg, "opMulAssign", opMulAssign<T, const T&>);
			if constexpr ((!is_matrix_v<T> || std::is_floating_point_v<typename T::value_type>) && !is_quaternion_v<T>)
				bind_func(reg, "opDivAssign", opDivAssign<T, const T&>);

			bind_func(reg, "opAdd", opAdd<T, const T&>);
			bind_func(reg, "opSub", opSub<T, const T&>);
			bind_func(reg, "opMul", opMul<T, const T&>);
			if constexpr ((!is_matrix_v<T> || std::is_floating_point_v<typename T::value_type>) && !is_quaternion_v<T>)
				bind_func(reg, "opDiv", opDiv<T, const T&>);

			if constexpr (!is_quaternion_v<T>)
			{
				bind_func(reg, "opAddAssign", opAddAssign<T, Value>);
				bind_func(reg, "opSubAssign", opSubAssign<T, Value>);
			}

			bind_func(reg, "opMulAssign", opMulAssign<T, Value>);
			bind_func(reg, "opDivAssign", opDivAssign<T, Value>);

			if constexpr (!is_quaternion_v<T>)
			{
				bind_func(reg, "opAdd", opAdd<T, Value>);
				bind_func(reg, "opSub", opSub<T, Value>);
			}

			bind_func(reg, "opMul", opMul<T, Value>);
			bind_func(reg, "opDiv", opDiv<T, Value>);

			if constexpr (is_vector_v<T> && T::length() > 1)
			{
				constexpr glm::length_t len = T::length();
				using Matrix                = glm::mat<len, len, typename T::value_type>;

				bind_func(reg, "opMul", opMul<T, const Matrix&>);
				if constexpr (std::is_floating_point_v<Value>)
					bind_func(reg, "opDiv", opDiv<T, const Matrix&>);
			}

			if constexpr (is_matrix_v<T>)
			{
				using Vector = typename T::col_type;
				bind_func(reg, "opAddAssign", opAddAssign<T, const Vector&>);
				bind_func(reg, "opSubAssign", opSubAssign<T, const Vector&>);
				bind_func(reg, "opMulAssign", opMulAssign<T, const Vector&>);
				bind_func(reg, "opDivAssign", opDivAssign<T, const Vector&>);
				bind_func(reg, "opMul", opMul<T, const Vector&>);

				if constexpr (std::is_floating_point_v<Value>)
					bind_func(reg, "opDiv", opDiv<T, const Vector&>);
			}
		}
	};

	template<>
	struct Operators<bool> : BooleanOperators {
	};

	template<typename Value>
		requires(std::is_integral_v<Value>)
	struct Operators<Value> : ScalarOperators {
		template<typename T>
		static T opCom(T& self)
		{
			return ~self;
		}

		template<typename T, typename Arg>
		static T& opShlAssign(T& self, Arg other)
		{
			return self <<= other;
		}

		template<typename T, typename Arg>
		static T& opShrAssign(T& self, Arg other)
		{
			return self >>= other;
		}

		template<typename T, typename Arg>
		static T opShl(const T& self, Arg other)
		{
			return self << other;
		}

		template<typename T, typename Arg>
		static T opShr(const T& self, Arg other)
		{
			return self >> other;
		}

		template<typename T, typename Arg>
		static T& opModAssign(T& self, Arg other)
		{
			return (self %= other);
		}

		template<typename T, typename Arg>
		static T opMod(T& self, Arg other)
		{
			return self % other;
		}

		template<typename T>
		static void bind(ScriptClassRegistrar* reg)
		{
			ScalarOperators::bind_ops<T>(reg);
			BooleanOperators::bind_ops<T>(reg, false);

			bind_func(reg, "opCom", opCom<T>);
			bind_func(reg, "opShlAssign", opShlAssign<T, const T&>);
			bind_func(reg, "opShrAssign", opShrAssign<T, const T&>);
			bind_func(reg, "opModAssign", opModAssign<T, const T&>);
			bind_func(reg, "opShlAssign", opShlAssign<T, Value>);
			bind_func(reg, "opShrAssign", opShrAssign<T, Value>);
			bind_func(reg, "opModAssign", opModAssign<T, Value>);

			bind_func(reg, "opShl", opShl<T, const T&>);
			bind_func(reg, "opShr", opShr<T, const T&>);
			bind_func(reg, "opMod", opMod<T, const T&>);
			bind_func(reg, "opShl", opShl<T, Value>);
			bind_func(reg, "opShr", opShr<T, Value>);
			bind_func(reg, "opMod", opMod<T, Value>);
		}
	};

	template<>
	struct Operators<float> : ScalarOperators {
		template<typename T>
		static T& opPowAssign(T& self, const T& other)
		{
			return (self = glm::pow(self, other));
		}

		template<typename T>
		static T& opPowAssignValue(T& self, T::value_type other)
		{
			return (self = glm::pow(self, T(other)));
		}

		template<typename T>
		static T opPow(T& self, const T& other)
		{
			return glm::pow(self, other);
		}

		template<typename T>
		static T opPowValue(T& self, T::value_type other)
		{
			return glm::pow(self, T(other));
		}

		template<typename T>
		static T& opModAssign(T& self, T other)
		{
			return (self = glm::modf(self, other));
		}

		template<typename T>
		static T& opModAssignValue(T& self, T::value_type other)
		{
			return opModAssign(self, T(other));
		}

		template<typename T>
		static T opMod(T& self, T other)
		{
			return glm::modf(self, other);
		}

		template<typename T>
		static T opModValue(T& self, T::value_type other)
		{
			return opMod(self, T(other));
		}

		template<typename T>
		static void bind_ops(ScriptClassRegistrar* reg)
		{
			ScalarOperators::bind_ops<T>(reg);

			if constexpr (is_vector_v<T>)
			{
				bind_func(reg, "opPowAssign", opPowAssign<T>);
				bind_func(reg, "opPowAssign", opPowAssignValue<T>);
				bind_func(reg, "opPow", opPow<T>);
				bind_func(reg, "opPow", opPowValue<T>);

				bind_func(reg, "opModAssign", opModAssign<T>);
				bind_func(reg, "opModAssign", opModAssignValue<T>);
				bind_func(reg, "opMod", opMod<T>);
				bind_func(reg, "opMod", opModValue<T>);
			}
		}
	};

	// VECTOR BINDINGS

	struct Vector1_Binder {
		template<typename Vector>
		static void bind_generic(ScriptClassRegistrar* reg)
		{
			String x = make_property<Vector>("x");
			String r = make_property<Vector>("r");
			String s = make_property<Vector>("s");

			reg->property(x.c_str(), &Vector::x);
			reg->property(r.c_str(), &Vector::r);
			reg->property(s.c_str(), &Vector::s);

			reg->static_function("int32 length()", &length_of<Vector>);
			bind_constructor<Vector, typename Vector::value_type>(reg);

			Operators<typename Vector::value_type>::template bind_ops<Vector>(reg);
		}

		template<typename Vector>
		static void bind(ScriptClassRegistrar* reg)
		{
			register_upcast<typename Vector::value_type>(reg);
		}
	};

	struct Vector2_Binder {
		template<typename Vector>
		static void bind_generic(ScriptClassRegistrar* reg)
		{
			Vector1_Binder::bind_generic<Vector>(reg);

			String y = make_property<Vector>("y");
			String g = make_property<Vector>("g");
			String t = make_property<Vector>("t");

			reg->property(y.c_str(), &Vector::y);
			reg->property(g.c_str(), &Vector::g);
			reg->property(t.c_str(), &Vector::t);

			register_upcast<typename up_cast<Vector>::Type>(reg);
		}

		template<typename Vector>
		static void bind(ScriptClassRegistrar* reg)
		{
			static_assert(Vector::length() == 2);
			using Vec1 = typename Vector::value_type;
			bind_constructor<Vector, Vec1, Vec1>(reg);
		}
	};

	struct Vector3_Binder {
	private:
		template<typename Vector, typename Redirect>
		static void bind_with_redirection(ScriptClassRegistrar* reg)
		{
			using Vec1 = typename Vector::value_type;
			using Vec2 = const redirect<typename up_cast<Vector>::Type, Redirect>::Type&;

			bind_constructor<Vector, Vec1, Vec2>(reg);
			bind_constructor<Vector, Vec2, Vec1>(reg);

			if constexpr (std::is_same_v<Redirect, Vec1>)
				bind_constructor<Vector, Vec1, Vec1, Vec1>(reg);
		}

	public:
		template<typename Vector>
		static void bind_generic(ScriptClassRegistrar* reg)
		{
			Vector2_Binder::bind_generic<Vector>(reg);

			String z = make_property<Vector>("z");
			String b = make_property<Vector>("b");
			String p = make_property<Vector>("p");

			reg->property(z.c_str(), &Vector::z);
			reg->property(b.c_str(), &Vector::b);
			reg->property(p.c_str(), &Vector::p);

			register_upcast<typename up_cast<Vector, 2>::Type>(reg);
		}

		template<typename Vector>
		static void bind(ScriptClassRegistrar* reg)
		{
			static_assert(Vector::length() == 3);

			bind_with_redirection<Vector, bool>(reg);
			bind_with_redirection<Vector, int32_t>(reg);
			bind_with_redirection<Vector, uint32_t>(reg);
			bind_with_redirection<Vector, float>(reg);
		}
	};

	struct Vector4_Binder {
	private:
		template<typename Vector, typename Redirect>
		static void bind_with_redirection(ScriptClassRegistrar* reg)
		{
			using Vec3 = const redirect<typename up_cast<Vector>::Type, Redirect>::Type&;
			using Vec2 = const redirect<typename up_cast<Vector, 2>::Type, Redirect>::Type&;
			using Vec1 = typename Vector::value_type;

			if constexpr (std::is_same_v<Vec1, Redirect>)
				bind_constructor<Vector, Vec1, Vec1, Vec1, Vec1>(reg);

			bind_constructor<Vector, Vec1, Vec1, Vec2>(reg);
			bind_constructor<Vector, Vec1, Vec2, Vec1>(reg);
			bind_constructor<Vector, Vec2, Vec1, Vec1>(reg);
			bind_constructor<Vector, Vec2, Vec2>(reg);
			bind_constructor<Vector, Vec3, Vec1>(reg);
			bind_constructor<Vector, Vec1, Vec3>(reg);
		}

	public:
		template<typename Vector>
		static void bind_generic(ScriptClassRegistrar* reg)
		{
			Vector3_Binder::bind_generic<Vector>(reg);

			String w = make_property<Vector>("w");
			String a = make_property<Vector>("a");
			String q = make_property<Vector>("q");

			reg->property(w.c_str(), &Vector::w);
			reg->property(a.c_str(), &Vector::a);
			reg->property(q.c_str(), &Vector::q);

			register_upcast<typename up_cast<Vector, 3>::Type>(reg);
		}

		template<typename Vector>
		static void bind(ScriptClassRegistrar* reg)
		{
			static_assert(Vector::length() == 4);
			bind_with_redirection<Vector, bool>(reg);
			bind_with_redirection<Vector, int32_t>(reg);
			bind_with_redirection<Vector, uint32_t>(reg);
			bind_with_redirection<Vector, float>(reg);
		}
	};

	static void register_types()
	{
		glm_element_types::for_each([]<typename T>() {
			create_registrar<glm::vec<1, T>>();
			create_registrar<glm::vec<2, T>>();
			create_registrar<glm::vec<3, T>>();
			create_registrar<glm::vec<4, T>>();
		});

		glm_element_types::for_each([]<typename T>() {
			create_registrar<glm::mat<2, 2, T>>();
			create_registrar<glm::mat<3, 3, T>>();
			create_registrar<glm::mat<4, 4, T>>();
		});

		create_registrar<Quaternion>();
	}

	static void bind_vector_types()
	{
		glm_element_types::for_each([]<typename T>() {
			bind_glm_type<glm::vec<1, T>, Vector1_Binder>();
			bind_glm_type<glm::vec<2, T>, Vector2_Binder>();
			bind_glm_type<glm::vec<3, T>, Vector3_Binder>();
			bind_glm_type<glm::vec<4, T>, Vector4_Binder>();
		});
	}

	// MATRIX BINDINGS
	struct Matrix_Binder {
		template<typename T>
		static void bind_generic(ScriptClassRegistrar* reg)
		{
			bind_constructor<T, typename T::value_type>(reg);

			glm_element_types::for_each([r = reg]<typename E>() {
				bind_constructor<T, glm::mat<2, 2, E>>(r);
				bind_constructor<T, glm::mat<3, 3, E>>(r);
				bind_constructor<T, glm::mat<4, 4, E>>(r);
			});

			reg->static_function("int32 length()", &length_of<T>);

			Operators<typename T::value_type>::template bind_ops<T>(reg);
		}
	};

	struct Matrix2_Binder : Matrix_Binder {
		template<typename Matrix>
		static void bind(ScriptClassRegistrar* reg)
		{
			using Column = typename Matrix::col_type;
			using Val    = typename Matrix::value_type;
			bind_constructor<Matrix,  //
							 Val, Val,//
							 Val, Val>(reg);
			bind_constructor<Matrix, const Column&, const Column&>(reg);
		}
	};

	struct Matrix3_Binder : Matrix_Binder {
		template<typename Matrix>
		static void bind(ScriptClassRegistrar* reg)
		{
			using Column = typename Matrix::col_type;
			using Val    = typename Matrix::value_type;
			bind_constructor<Matrix,       //
							 Val, Val, Val,//
							 Val, Val, Val,//
							 Val, Val, Val>(reg);
			bind_constructor<Matrix, const Column&, const Column&, const Column&>(reg);
		}
	};

	struct Matrix4_Binder : Matrix_Binder {
		template<typename Matrix>
		static void bind(ScriptClassRegistrar* reg)
		{
			using Column = typename Matrix::col_type;
			using Val    = typename Matrix::value_type;
			bind_constructor<Matrix,            //
							 Val, Val, Val, Val,//
							 Val, Val, Val, Val,//
							 Val, Val, Val, Val,//
							 Val, Val, Val, Val>(reg);
			bind_constructor<Matrix, const Column&, const Column&, const Column&, const Column&>(reg);
		}
	};

	static void bind_matrix_types()
	{
		glm_element_types::for_each([]<typename T>() {
			bind_glm_type<glm::mat<2, 2, T>, Matrix2_Binder>();
			bind_glm_type<glm::mat<3, 3, T>, Matrix3_Binder>();
			bind_glm_type<glm::mat<4, 4, T>, Matrix4_Binder>();
		});
	}

	static void bind_quaternion()
	{
		auto quat = exiting_class<Quaternion>();
		quat.property("float x", &Quaternion::x);
		quat.property("float y", &Quaternion::y);
		quat.property("float z", &Quaternion::z);
		quat.property("float w", &Quaternion::w);

		bind_constructor<Quaternion, float, float, float, float>(&quat);

		glm_element_types::for_each([&quat]<typename T>() {
			using Vec3 = glm::vec<3, T>;
			using Mat3 = glm::mat<3, 3, T>;
			using Mat4 = glm::mat<4, 4, T>;

			bind_constructor<Quaternion, Vec3>(&quat);
			bind_constructor<Quaternion, T, Vec3>(&quat);
			bind_constructor<Quaternion, Vec3, Vec3>(&quat);
			bind_constructor<Quaternion, Mat3>(&quat);
			bind_constructor<Quaternion, Mat4>(&quat);
		});

		quat.static_function("int32 length()", &length_of<Quaternion>);

		Operators<float>::template bind_ops<Quaternion>(&quat);
	}

	// GLM FUNCTIONS

	static void bind_functions()
	{
		ScriptNamespaceScopedChanger changer("glm");

		bind_func("sin", func_of<float, float>(glm::sin));
		bind_func("sin", glm::sin<1, float, glm::defaultp>);
		bind_func("sin", glm::sin<2, float, glm::defaultp>);
		bind_func("sin", glm::sin<3, float, glm::defaultp>);
		bind_func("sin", glm::sin<4, float, glm::defaultp>);

		bind_func("cos", func_of<float, float>(glm::cos));
		bind_func("cos", glm::cos<1, float, glm::defaultp>);
		bind_func("cos", glm::cos<2, float, glm::defaultp>);
		bind_func("cos", glm::cos<3, float, glm::defaultp>);
		bind_func("cos", glm::cos<4, float, glm::defaultp>);

		bind_func("tan", func_of<float, float>(glm::tan));
		bind_func("tan", glm::tan<1, float, glm::defaultp>);
		bind_func("tan", glm::tan<2, float, glm::defaultp>);
		bind_func("tan", glm::tan<3, float, glm::defaultp>);
		bind_func("tan", glm::tan<4, float, glm::defaultp>);

		bind_func("asin", func_of<float, float>(glm::asin));
		bind_func("asin", glm::asin<1, float, glm::defaultp>);
		bind_func("asin", glm::asin<2, float, glm::defaultp>);
		bind_func("asin", glm::asin<3, float, glm::defaultp>);
		bind_func("asin", glm::asin<4, float, glm::defaultp>);

		bind_func("acos", func_of<float, float>(glm::acos));
		bind_func("acos", glm::acos<1, float, glm::defaultp>);
		bind_func("acos", glm::acos<2, float, glm::defaultp>);
		bind_func("acos", glm::acos<3, float, glm::defaultp>);
		bind_func("acos", glm::acos<4, float, glm::defaultp>);

		bind_func("atan", func_of<float, float, float>(glm::atan));
		bind_func("atan", func_of<Vector1f, const Vector1f&, const Vector1f&>(glm::atan));
		bind_func("atan", func_of<Vector2f, const Vector2f&, const Vector2f&>(glm::atan));
		bind_func("atan", func_of<Vector3f, const Vector3f&, const Vector3f&>(glm::atan));
		bind_func("atan", func_of<Vector4f, const Vector4f&, const Vector4f&>(glm::atan));

		bind_func("sinh", func_of<float, float>(glm::sinh));
		bind_func("sinh", glm::sinh<1, float, glm::defaultp>);
		bind_func("sinh", glm::sinh<2, float, glm::defaultp>);
		bind_func("sinh", glm::sinh<3, float, glm::defaultp>);
		bind_func("sinh", glm::sinh<4, float, glm::defaultp>);

		bind_func("cosh", func_of<float, float>(glm::cosh));
		bind_func("cosh", glm::cosh<1, float, glm::defaultp>);
		bind_func("cosh", glm::cosh<2, float, glm::defaultp>);
		bind_func("cosh", glm::cosh<3, float, glm::defaultp>);
		bind_func("cosh", glm::cosh<4, float, glm::defaultp>);

		bind_func("tanh", func_of<float, float>(glm::tanh));
		bind_func("tanh", glm::tanh<1, float, glm::defaultp>);
		bind_func("tanh", glm::tanh<2, float, glm::defaultp>);
		bind_func("tanh", glm::tanh<3, float, glm::defaultp>);
		bind_func("tanh", glm::tanh<4, float, glm::defaultp>);

		bind_func("log", func_of<float, float>(glm::log));
		bind_func("log", glm::log<1, float, glm::defaultp>);
		bind_func("log", glm::log<2, float, glm::defaultp>);
		bind_func("log", glm::log<3, float, glm::defaultp>);
		bind_func("log", glm::log<4, float, glm::defaultp>);

		bind_func("log2", func_of<float, float>(glm::log2));
		bind_func("log2", glm::log2<1, float, glm::defaultp>);
		bind_func("log2", glm::log2<2, float, glm::defaultp>);
		bind_func("log2", glm::log2<3, float, glm::defaultp>);
		bind_func("log2", glm::log2<4, float, glm::defaultp>);

		bind_func("pow", func_of<float, float, float>(glm::pow));
		bind_func("pow", glm::pow<1, float, glm::defaultp>);
		bind_func("pow", glm::pow<2, float, glm::defaultp>);
		bind_func("pow", glm::pow<3, float, glm::defaultp>);
		bind_func("pow", glm::pow<4, float, glm::defaultp>);

		bind_func("exp", func_of<float, float>(glm::exp));
		bind_func("exp", glm::exp<1, float, glm::defaultp>);
		bind_func("exp", glm::exp<2, float, glm::defaultp>);
		bind_func("exp", glm::exp<3, float, glm::defaultp>);
		bind_func("exp", glm::exp<4, float, glm::defaultp>);

		bind_func("sqrt", func_of<float, float>(glm::sqrt));
		bind_func("sqrt", glm::sqrt<1, float, glm::defaultp>);
		bind_func("sqrt", glm::sqrt<2, float, glm::defaultp>);
		bind_func("sqrt", glm::sqrt<3, float, glm::defaultp>);
		bind_func("sqrt", glm::sqrt<4, float, glm::defaultp>);

		bind_func("ceil", func_of<float, float>(glm::ceil));
		bind_func("ceil", glm::ceil<1, float, glm::defaultp>);
		bind_func("ceil", glm::ceil<2, float, glm::defaultp>);
		bind_func("ceil", glm::ceil<3, float, glm::defaultp>);
		bind_func("ceil", glm::ceil<4, float, glm::defaultp>);

		bind_func("abs", func_of<float, float>(glm::abs));
		bind_func("abs", glm::abs<1, float, glm::defaultp>);
		bind_func("abs", glm::abs<2, float, glm::defaultp>);
		bind_func("abs", glm::abs<3, float, glm::defaultp>);
		bind_func("abs", glm::abs<4, float, glm::defaultp>);

		bind_func("floor", func_of<float, float>(glm::floor));
		bind_func("floor", glm::floor<1, float, glm::defaultp>);
		bind_func("floor", glm::floor<2, float, glm::defaultp>);
		bind_func("floor", glm::floor<3, float, glm::defaultp>);
		bind_func("floor", glm::floor<4, float, glm::defaultp>);

		bind_func("dot", glm::dot<float>);
		bind_func("dot", glm::dot<1, float, glm::defaultp>);
		bind_func("dot", glm::dot<2, float, glm::defaultp>);
		bind_func("dot", glm::dot<3, float, glm::defaultp>);
		bind_func("dot", glm::dot<4, float, glm::defaultp>);

		bind_func("cross", func_of<Vector3f, const Vector3f&, const Vector3f&>(glm::cross));
		bind_func("cross", func_of<Quaternion, const Quaternion&, const Quaternion&>(glm::cross));

		bind_func("length", glm::length<float>);
		bind_func("length", glm::length<1, float, glm::defaultp>);
		bind_func("length", glm::length<2, float, glm::defaultp>);
		bind_func("length", glm::length<3, float, glm::defaultp>);
		bind_func("length", glm::length<4, float, glm::defaultp>);

		bind_func("normalize", glm::normalize<1, float, glm::defaultp>);
		bind_func("normalize", glm::normalize<2, float, glm::defaultp>);
		bind_func("normalize", glm::normalize<3, float, glm::defaultp>);
		bind_func("normalize", glm::normalize<4, float, glm::defaultp>);

		bind_func("distance", glm::distance<float>);
		bind_func("distance", glm::distance<1, float, glm::defaultp>);
		bind_func("distance", glm::distance<2, float, glm::defaultp>);
		bind_func("distance", glm::distance<3, float, glm::defaultp>);
		bind_func("distance", glm::distance<4, float, glm::defaultp>);

		bind_func("reflect", glm::reflect<float>);
		bind_func("reflect", glm::reflect<1, float, glm::defaultp>);
		bind_func("reflect", glm::reflect<2, float, glm::defaultp>);
		bind_func("reflect", glm::reflect<3, float, glm::defaultp>);
		bind_func("reflect", glm::reflect<4, float, glm::defaultp>);

		bind_func("mix", glm::mix<float, float>);
		bind_func("mix", func_of<Vector1f, const Vector1f&, const Vector1f&, float>(glm::mix));
		bind_func("mix", func_of<Vector2f, const Vector2f&, const Vector2f&, float>(glm::mix));
		bind_func("mix", func_of<Vector3f, const Vector3f&, const Vector3f&, float>(glm::mix));
		bind_func("mix", func_of<Vector4f, const Vector4f&, const Vector4f&, float>(glm::mix));
		bind_func("mix", func_of<Vector2f, const Vector2f&, const Vector2f&, const Vector2f&>(glm::mix));
		bind_func("mix", func_of<Vector3f, const Vector3f&, const Vector3f&, const Vector3f&>(glm::mix));
		bind_func("mix", func_of<Vector4f, const Vector4f&, const Vector4f&, const Vector4f&>(glm::mix));
		bind_func("mix", func_of<Quaternion, const Quaternion&, const Quaternion&, float>(glm::mix));

		bind_func("max", glm::max<float>);
		bind_func("max", func_of<Vector1f, const Vector1f&, float>(glm::max));
		bind_func("max", func_of<Vector2f, const Vector2f&, float>(glm::max));
		bind_func("max", func_of<Vector3f, const Vector3f&, float>(glm::max));
		bind_func("max", func_of<Vector4f, const Vector4f&, float>(glm::max));
		bind_func("max", func_of<Vector1f, const Vector1f&, const Vector1f&>(glm::max));
		bind_func("max", func_of<Vector2f, const Vector2f&, const Vector2f&>(glm::max));
		bind_func("max", func_of<Vector3f, const Vector3f&, const Vector3f&>(glm::max));
		bind_func("max", func_of<Vector4f, const Vector4f&, const Vector4f&>(glm::max));

		bind_func("min", glm::min<float>);
		bind_func("min", func_of<Vector1f, const Vector1f&, float>(glm::min));
		bind_func("min", func_of<Vector2f, const Vector2f&, float>(glm::min));
		bind_func("min", func_of<Vector3f, const Vector3f&, float>(glm::min));
		bind_func("min", func_of<Vector4f, const Vector4f&, float>(glm::min));
		bind_func("min", func_of<Vector1f, const Vector1f&, const Vector1f&>(glm::min));
		bind_func("min", func_of<Vector2f, const Vector2f&, const Vector2f&>(glm::min));
		bind_func("min", func_of<Vector3f, const Vector3f&, const Vector3f&>(glm::min));
		bind_func("min", func_of<Vector4f, const Vector4f&, const Vector4f&>(glm::min));

		bind_func("clamp", glm::clamp<float>);
		bind_func("clamp", func_of<Vector1f, const Vector1f&, float, float>(glm::clamp));
		bind_func("clamp", func_of<Vector2f, const Vector2f&, float, float>(glm::clamp));
		bind_func("clamp", func_of<Vector3f, const Vector3f&, float, float>(glm::clamp));
		bind_func("clamp", func_of<Vector4f, const Vector4f&, float, float>(glm::clamp));
		bind_func("clamp", func_of<Vector1f, const Vector1f&, const Vector1f&, const Vector1f&>(glm::clamp));
		bind_func("clamp", func_of<Vector2f, const Vector2f&, const Vector2f&, const Vector2f&>(glm::clamp));
		bind_func("clamp", func_of<Vector3f, const Vector3f&, const Vector3f&, const Vector3f&>(glm::clamp));
		bind_func("clamp", func_of<Vector4f, const Vector4f&, const Vector4f&, const Vector4f&>(glm::clamp));

		bind_func("lerp", func_of<Quaternion, const Quaternion&, const Quaternion&, float>(glm::lerp));
		bind_func("slerp", func_of<Quaternion, const Quaternion&, const Quaternion&, float>(glm::slerp));

		bind_func("angle", glm::angle<float, glm::defaultp>);

		bind_func("inverse", glm::inverse<2, 2, float, glm::defaultp>);
		bind_func("inverse", glm::inverse<3, 3, float, glm::defaultp>);
		bind_func("inverse", glm::inverse<4, 4, float, glm::defaultp>);

		bind_func("determinant", glm::determinant<2, 2, float, glm::defaultp>);
		bind_func("determinant", glm::determinant<3, 3, float, glm::defaultp>);
		bind_func("determinant", glm::determinant<4, 4, float, glm::defaultp>);

		glm_element_types::for_each([]<typename T>() {
			bind_func("transpose", glm::transpose<2, 2, T, glm::defaultp>);
			bind_func("transpose", glm::transpose<3, 3, T, glm::defaultp>);
			bind_func("transpose", glm::transpose<4, 4, T, glm::defaultp>);
		});
	}

	static void bind()
	{
		register_types();
		bind_vector_types();
		bind_matrix_types();
		bind_quaternion();
		bind_functions();
	}

	static PreInitializeController bindings(bind, "glm");
}// namespace Engine::Bindings::GLM
