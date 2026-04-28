#pragma once
#include <functional>

namespace Trinex
{
	template<typename Signature>
	using Function = std::function<Signature>;

	template<typename>
	class FunctionRef;

	template<typename R, typename... Args>
	class FunctionRef<R(Args...)>
	{
	private:
		void* m_object                = nullptr;
		R (*m_caller)(void*, Args...) = nullptr;

	public:
		FunctionRef() = default;

		template<typename F>
		    requires(!std::same_as<std::remove_cvref_t<F>, FunctionRef<R(Args...)>> && std::is_invocable_r_v<R, F&, Args...>)
		FunctionRef(F&& f) noexcept
		    : m_object(const_cast<void*>(static_cast<const void*>(std::addressof(f)))),
		      m_caller([](void* object, Args... args) -> R {
			      using Fn = std::remove_reference_t<F>;

			      if constexpr (std::is_void_v<R>)
			      {
				      std::invoke(*static_cast<Fn*>(object), std::forward<Args>(args)...);
			      }
			      else
			      {
				      return std::invoke(*static_cast<Fn*>(object), std::forward<Args>(args)...);
			      }
		      })
		{}

		R operator()(Args... args) const { return m_caller(m_object, std::forward<Args>(args)...); }
		explicit operator bool() const noexcept { return m_caller != nullptr; }
	};
}// namespace Trinex
