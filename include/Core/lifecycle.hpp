#pragma once
#include <initializer_list>

namespace Trinex::LifeCycle
{
	using Function = void (*)();

	enum Stage
	{
		PreInit          = -11,
		ReflectionInit   = -10,
		CoreRegistration = -9,
		ConfigsInit      = -8,
		CoreInit         = -7,
		Init             = -6,
		ResourcesInit    = -5,
		PostInit         = -4,
		PreShutdown      = -3,
		Shutdown         = -2,
		PostShutdown     = -1,
	};

	struct Description
	{
		const char* name                         = "Undefined";
		std::initializer_list<const char*> after = {};
	};

	class trinex_api Registrar
	{
	public:
		Registrar(i32 stage, Function function, const Description& description = {});
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

	trinex_api void execute(i32 stage);
} // namespace Trinex::LifeCycle

#define trinex_lifecycle_handler(stage_id, ...)                                                                                  \
	static void TRINEX_CONCAT(trinex_lifecycle_function_, __LINE__)();                                                           \
                                                                                                                                 \
	static const ::Trinex::LifeCycle::Registrar TRINEX_CONCAT(trinex_lifecycle_registrar_, __LINE__)(                            \
	        stage_id, &TRINEX_CONCAT(trinex_lifecycle_function_, __LINE__) __VA_OPT__(, ) __VA_ARGS__);                          \
                                                                                                                                 \
	static void TRINEX_CONCAT(trinex_lifecycle_function_, __LINE__)()

#define trinex_on_pre_init(...) trinex_lifecycle_handler(::Trinex::LifeCycle::Stage::PreInit, __VA_ARGS__)
#define trinex_on_reflection_init(...) trinex_lifecycle_handler(::Trinex::LifeCycle::Stage::ReflectionInit, __VA_ARGS__)
#define trinex_on_core_registration(...) trinex_lifecycle_handler(::Trinex::LifeCycle::Stage::CoreRegistration, __VA_ARGS__)
#define trinex_on_configs_init(...) trinex_lifecycle_handler(::Trinex::LifeCycle::Stage::ConfigsInit, __VA_ARGS__)
#define trinex_on_core_init(...) trinex_lifecycle_handler(::Trinex::LifeCycle::Stage::CoreInit, __VA_ARGS__)
#define trinex_on_init(...) trinex_lifecycle_handler(::Trinex::LifeCycle::Stage::Init, __VA_ARGS__)
#define trinex_on_resources_init(...) trinex_lifecycle_handler(::Trinex::LifeCycle::Stage::ResourcesInit, __VA_ARGS__)
#define trinex_on_post_init(...) trinex_lifecycle_handler(::Trinex::LifeCycle::Stage::PostInit, __VA_ARGS__)
#define trinex_on_pre_shutdown(...) trinex_lifecycle_handler(::Trinex::LifeCycle::Stage::PreShutdown, __VA_ARGS__)
#define trinex_on_shutdown(...) trinex_lifecycle_handler(::Trinex::LifeCycle::Stage::Shutdown, __VA_ARGS__)
#define trinex_on_post_shutdown(...) trinex_lifecycle_handler(::Trinex::LifeCycle::Stage::PostShutdown, __VA_ARGS__)
