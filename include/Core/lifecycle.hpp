#pragma once
#include <initializer_list>

namespace Trinex::LifeCycle
{
	using Function = void (*)();

	enum Stage
	{
		// Startup
		Bootstrap  = -9,
		Register   = -8,
		Configure  = -7,
		Initialize = -6,
		Load       = -5,
		Start      = -4,

		// Shutdown
		Stop     = -3,
		Shutdown = -2,
		Finalize = -1
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

	static inline Registrar on_bootstrap(Function function, const Description& description = {})
	{
		return Registrar(Bootstrap, function, description);
	}

	static inline Registrar on_register(Function function, const Description& description = {})
	{
		return Registrar(Register, function, description);
	}

	static inline Registrar on_configure(Function function, const Description& description = {})
	{
		return Registrar(Configure, function, description);
	}

	static inline Registrar on_initialize(Function function, const Description& description = {})
	{
		return Registrar(Initialize, function, description);
	}

	static inline Registrar on_load(Function function, const Description& description = {})
	{
		return Registrar(Load, function, description);
	}

	static inline Registrar on_start(Function function, const Description& description = {})
	{
		return Registrar(Start, function, description);
	}

	static inline Registrar on_stop(Function function, const Description& description = {})
	{
		return Registrar(Stop, function, description);
	}

	static inline Registrar on_shutdown(Function function, const Description& description = {})
	{
		return Registrar(Shutdown, function, description);
	}

	static inline Registrar on_finalize(Function function, const Description& description = {})
	{
		return Registrar(Finalize, function, description);
	}

	trinex_api void execute(i32 stage);
} // namespace Trinex::LifeCycle

#define trinex_lifecycle_handler(stage_id, ...)                                                                                  \
	static void TRINEX_CONCAT(trinex_lifecycle_function_, __LINE__)();                                                           \
                                                                                                                                 \
	static const ::Trinex::LifeCycle::Registrar TRINEX_CONCAT(trinex_lifecycle_registrar_, __LINE__)(                            \
	        stage_id, &TRINEX_CONCAT(trinex_lifecycle_function_, __LINE__), {__VA_ARGS__});                                      \
                                                                                                                                 \
	static void TRINEX_CONCAT(trinex_lifecycle_function_, __LINE__)()

#define trinex_on_bootstrap(...) trinex_lifecycle_handler(::Trinex::LifeCycle::Stage::Bootstrap, __VA_ARGS__)
#define trinex_on_register(...) trinex_lifecycle_handler(::Trinex::LifeCycle::Stage::Register, __VA_ARGS__)
#define trinex_on_configure(...) trinex_lifecycle_handler(::Trinex::LifeCycle::Stage::Configure, __VA_ARGS__)
#define trinex_on_initialize(...) trinex_lifecycle_handler(::Trinex::LifeCycle::Stage::Initialize, __VA_ARGS__)
#define trinex_on_load(...) trinex_lifecycle_handler(::Trinex::LifeCycle::Stage::Load, __VA_ARGS__)
#define trinex_on_start(...) trinex_lifecycle_handler(::Trinex::LifeCycle::Stage::Start, __VA_ARGS__)
#define trinex_on_stop(...) trinex_lifecycle_handler(::Trinex::LifeCycle::Stage::Stop, __VA_ARGS__)
#define trinex_on_shutdown(...) trinex_lifecycle_handler(::Trinex::LifeCycle::Stage::Shutdown, __VA_ARGS__)
#define trinex_on_finalize(...) trinex_lifecycle_handler(::Trinex::LifeCycle::Stage::Finalize, __VA_ARGS__)
