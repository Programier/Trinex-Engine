#pragma once
#include <Core/engine_types.hpp>
#include <Core/string_functions.hpp>
#include <ScriptEngine/script_engine.hpp>

namespace Engine
{
	// Wrappers
	template<typename T, typename GLM>
	static T glm_normalize(const T& v)
	{
		return glm::normalize(static_cast<const GLM&>(v));
	}

	template<typename T, typename GLM>
	static typename T::value_type glm_length(const T& v)
	{
		return glm::length(static_cast<const GLM&>(v));
	}

	template<typename T, typename GLM>
	static typename T::value_type glm_dot(const T& v1, const T& v2)
	{
		return glm::dot(static_cast<const GLM&>(v1), static_cast<const GLM&>(v2));
	}

	template<typename T, typename GLM>
	static T glm_cross(const T& v1, const T& v2)
	{
		return glm::cross(static_cast<const GLM&>(v1), static_cast<const GLM&>(v2));
	}

	template<typename T, typename GLM>
	static T glm_max(const T& a, const T& b)
	{
		return glm::max(static_cast<const GLM&>(a), static_cast<const GLM&>(b));
	}

	template<typename T, typename GLM>
	static T glm_min(const T& a, const T& b)
	{
		return glm::min(static_cast<const GLM&>(a), static_cast<const GLM&>(b));
	}

	template<typename T, typename GLM>
	static T glm_clamp(const T& a, const T& b, const T& c)
	{
		return glm::clamp(static_cast<const GLM&>(a), static_cast<const GLM&>(b), static_cast<const GLM&>(c));
	}

	template<typename T, typename B, typename GLM>
	static T glm_clamp(const T& a, B b, B c)
	{
		return glm::clamp(static_cast<const GLM&>(a), b, c);
	}

	// Bindings

	enum BindFlags
	{
		Normalize = (1 << 0),
		Length    = (1 << 1),
		Dot       = (1 << 2),
		Cross     = (1 << 3),
		Max       = (1 << 4),
		Min       = (1 << 5),
		Clamp     = (1 << 6),
		All       = ~size_t(0),
	};

#define checked_bind(flag, code)                                                                                                 \
	if constexpr (((flags) & (flag)))                                                                                            \
	{                                                                                                                            \
		code;                                                                                                                    \
	}

	template<typename T, typename GLM, size_t flags = All>
	void bind_wrapped_functions(const String& name, const String& vtype)
	{
		ScriptNamespaceScopedChanger saver("glm");
#define regf ScriptEngine::register_function


		checked_bind(Normalize, regf(Strings::format("{} normalize(const {}& in)", name, name), glm_normalize<T, GLM>));
		checked_bind(Length, regf(Strings::format("{} length(const {}& in)", vtype, name), glm_length<T, GLM>));
		checked_bind(Dot, regf(Strings::format("{} dot(const {}& in, const {}& in)", vtype, name, name), glm_dot<T, GLM>));
		checked_bind(Cross, regf(Strings::format("{} cross(const {}& in, const {}& in)", name, name, name), glm_cross<T, GLM>));
		checked_bind(Max, regf(Strings::format("{} max(const {}& in, const {}& in)", name, name, name), glm_max<T, GLM>));
		checked_bind(Min, regf(Strings::format("{} min(const {}& in, const {}& in)", name, name, name), glm_min<T, GLM>));
		checked_bind(Clamp, regf(Strings::format("{} clamp(const {}& in, const {}& in, const {}& in)", name, name, name, name),
		                         glm_clamp<T, GLM>));
		checked_bind(Clamp, regf(Strings::format("{} clamp(const {}& in, {}, {})", name, name, vtype, vtype),
		                         glm_clamp<T, typename T::value_type, GLM>));
	}
}// namespace Engine
