#pragma once

#include <Core/definitions.hpp>
#include <Core/etl/utility.hpp>
#include <Core/object.hpp>


namespace Trinex
{
	struct EmptySingletoneParent {
	};

	template<typename Type, typename Parent = EmptySingletoneParent, bool with_destroy_controller = true>
	class Singletone : public Parent
	{
	public:
		INLINE_DEBUGGABLE static Type* instance()
		{
			if (Type::s_instance == nullptr)
			{
				Type::s_instance = trx_new Type();

				if constexpr (with_destroy_controller)
				{
					LifeCycle::on_post_shutdown([]() {
						if (Type::s_instance)
						{
							trx_delete_inline(Type::s_instance);
						}
					});
				}
			}

			return Type::s_instance;
		}

		~Singletone() { Type::s_instance = nullptr; }
	};
}// namespace Trinex
