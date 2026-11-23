export module Trinex.Core.LifeCycle;
export import Trinex.Core.ETL;

export namespace Trinex::LifeCycle
{
	using Function = Delegate<void()>;

	enum Stage
	{
		// Testing
		Testing = -11,

		// Startup
		Bootstrap  = -10,
		Register   = -9,
		Configure  = -8,
		Initialize = -7,
		Load       = -6,
		Start      = -5,

		// Shutdown
		Stop     = -4,
		Shutdown = -3,
		Teardown = -2,
		Finalize = -1
	};

	struct Description
	{
		StringView name                  = "Undefined";
		InitializerList<StringView> deps = {};
	};

	class trinex_api Registrar
	{
	public:
		Registrar(i32 stage, Function function, const Description& description = {});
	};

	inline Registrar on_bootstrap(Function function, const Description& description = {})
	{
		return Registrar(Bootstrap, function, description);
	}

	inline Registrar on_register(Function function, const Description& description = {})
	{
		return Registrar(Register, function, description);
	}

	inline Registrar on_configure(Function function, const Description& description = {})
	{
		return Registrar(Configure, function, description);
	}

	inline Registrar on_initialize(Function function, const Description& description = {})
	{
		return Registrar(Initialize, function, description);
	}

	inline Registrar on_load(Function function, const Description& description = {})
	{
		return Registrar(Load, function, description);
	}

	inline Registrar on_start(Function function, const Description& description = {})
	{
		return Registrar(Start, function, description);
	}

	inline Registrar on_stop(Function function, const Description& description = {})
	{
		return Registrar(Stop, function, description);
	}

	inline Registrar on_shutdown(Function function, const Description& description = {})
	{
		return Registrar(Shutdown, function, description);
	}

	inline Registrar on_teardown(Function function, const Description& description = {})
	{
		return Registrar(Teardown, function, description);
	}

	inline Registrar on_finalize(Function function, const Description& description = {})
	{
		return Registrar(Finalize, function, description);
	}

	trinex_api void execute(i32 stage);
	trinex_api void execute(i32 stage, StringView handler);
} // namespace Trinex::LifeCycle
