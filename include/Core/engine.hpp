#pragma once
#include <Core/arguments.hpp>
#include <Core/base_engine.hpp>
#include <Core/constants.hpp>
#include <Core/engine_types.hpp>
#include <Core/structures.hpp>

namespace Engine
{
	class Thread;
	class Window;

	struct RHI;

	class ENGINE_EXPORT EngineInstance : public BaseEngine
	{
		declare_class(EngineInstance, BaseEngine);

	private:
	public:
		ENGINE_EXPORT static const String& project_name();
		ENGINE_EXPORT static const String& project_name(const String& name);

		int_t init() override;
		int_t terminate() override;
	};
}// namespace Engine
