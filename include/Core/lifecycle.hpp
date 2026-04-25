#pragma once
#include <initializer_list>

namespace Trinex::LifeCycle
{
	using Function = void (*)();

	enum Event
	{
		PreInit        = -7,
		ReflectionInit = -6,
		Init           = -5,
		ConfigsInit    = -4,
		ResourcesInit  = -3,
		Shutdown       = -2,
		PostShutdown   = -1,
	};

	struct Description {
		const char* name                         = "Undefined";
		std::initializer_list<const char*> after = {};
	};

	class ENGINE_EXPORT Registrar
	{
	public:
		Registrar(i32 event, Function function, const Description& description = {});
	};

	static inline Registrar on_pre_init(Function function, const Description& description = {})
	{
		return Registrar(PreInit, function, description);
	}

	static inline Registrar on_reflection_init(Function function, const Description& description = {})
	{
		return Registrar(ReflectionInit, function, description);
	}

	static inline Registrar on_init(Function function, const Description& description = {})
	{
		return Registrar(Init, function, description);
	}

	static inline Registrar on_configs_init(Function function, const Description& description = {})
	{
		return Registrar(ConfigsInit, function, description);
	}

	static inline Registrar on_resources_init(Function function, const Description& description = {})
	{
		return Registrar(ResourcesInit, function, description);
	}

	static inline Registrar on_shutdown(Function function, const Description& description = {})
	{
		return Registrar(Shutdown, function, description);
	}

	static inline Registrar on_post_shutdown(Function function, const Description& description = {})
	{
		return Registrar(PostShutdown, function, description);
	}

	ENGINE_EXPORT void execute(i32 event);
}// namespace Trinex::LifeCycle

#define trinex_lifecycle_handler(event_id, ...)                                                                                  \
	static void TRINEX_CONCAT(trinex_lifecycle_function_, __LINE__)();                                                           \
                                                                                                                                 \
	static const ::Trinex::LifeCycle::Registrar TRINEX_CONCAT(trinex_lifecycle_registrar_, __LINE__)(                            \
	        event_id, &TRINEX_CONCAT(trinex_lifecycle_function_, __LINE__) __VA_OPT__(, ) __VA_ARGS__);                          \
                                                                                                                                 \
	static void TRINEX_CONCAT(trinex_lifecycle_function_, __LINE__)()

#define trinex_on_pre_init(...) trinex_lifecycle_handler(::Trinex::LifeCycle::Event::PreInit, __VA_ARGS__)
#define trinex_on_reflection_init(...) trinex_lifecycle_handler(::Trinex::LifeCycle::Event::ReflectionInit, __VA_ARGS__)
#define trinex_on_init(...) trinex_lifecycle_handler(::Trinex::LifeCycle::Event::Init, __VA_ARGS__)
#define trinex_on_configs_init(...) trinex_lifecycle_handler(::Trinex::LifeCycle::Event::ConfigsInit, __VA_ARGS__)
#define trinex_on_resources_init(...) trinex_lifecycle_handler(::Trinex::LifeCycle::Event::ResourcesInit, __VA_ARGS__)
#define trinex_on_shutdown(...) trinex_lifecycle_handler(::Trinex::LifeCycle::Event::Shutdown, __VA_ARGS__)
#define trinex_on_post_shutdown(...) trinex_lifecycle_handler(::Trinex::LifeCycle::Event::PostShutdown, __VA_ARGS__)
