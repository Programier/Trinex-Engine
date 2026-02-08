#pragma once

namespace Engine
{
	template<typename T>
	class ScopeVariable
	{
		T& m_var;
		T m_prev;

	public:
		inline ScopeVariable(T& var, const T& new_value) : m_var(var), m_prev(var) { m_var = new_value; }
		inline ~ScopeVariable() { m_var = m_prev; }
	};
}// namespace Engine
