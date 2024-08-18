#include <Core/class.hpp>
#include <Core/engine.hpp>


namespace Engine
{
	implement_engine_class_default_init(EngineInstance, 0);

	ENGINE_EXPORT const String& EngineInstance::project_name()
	{
		static String name = "Trinex Engine";
		return name;
	}

	ENGINE_EXPORT const String& EngineInstance::project_name(const String& name)
	{
		const_cast<String&>(project_name()) = name;
		return project_name();
	}

	/////////////////// INITIALIZE ENGINE ///////////////////


	ENGINE_EXPORT int_t EngineInstance::init()
	{
		return Super::init();
	}

	ENGINE_EXPORT int_t EngineInstance::terminate()
	{
		return Super::terminate();
	}


}// namespace Engine
