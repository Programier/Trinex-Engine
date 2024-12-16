#pragma once
#include <Core/engine_types.hpp>

namespace Engine
{
	class ENGINE_EXPORT TaskInterface
	{
	public:
		virtual void execute()      = 0;
		virtual size_t size() const = 0;
		virtual ~TaskInterface();
	};

	template<typename TaskType, typename BaseType = TaskInterface>
	class Task : public BaseType
	{
	public:
		using BaseType::BaseType;

		size_t size() const override
		{
			return sizeof(TaskType);
		}
	};
}// namespace Engine
