#include <Core/command_line.hpp>
#include <Core/console.hpp>
#include <Core/etl/flat_map.hpp>
#include <Core/reflection/type.hpp>
#include <Platform/system.hpp>
#include <RHI/rhi.hpp>

namespace Trinex
{
	struct Backend
	{
		mutable RHI* backend = nullptr;
		RHI::CreateFunction create;
		RHI::DestroyFunction destroy;

		inline bool is_supported() const { return create && destroy; }
	};

	static trinex_console_var(RHIBackend, s_backend, "trinex.rhi.backend", RHIBackend::Undefined);

	static FlatMap<RHIBackend, Backend>& backends()
	{
		static FlatMap<RHIBackend, Backend> registry;
		return registry;
	}

	RHI::Registry::Registry(RHIBackend backend, CreateFunction create, DestroyFunction destroy)
	{
		auto [it, inserted] = backends().insert({backend, {nullptr, create, destroy}});
		trinex_verify_msg(inserted, "RHI backend is already registered");
	}

	bool RHI::is_supported(RHIBackend backend)
	{
		auto it = backends().find(backend);

		if (it != backends().end())
		{
			return it->second.is_supported();
		}

		return false;
	}

	RHIBackend RHI::backend()
	{
		static RHIBackend backend = []() -> RHIBackend {
			auto name = CommandLine::value("rhi");

			if (!name.empty())
			{
				RHIBackend backend = RHIBackend::Undefined;

				if (Refl::type_of<RHIBackend>()->parse(&backend, name))
				{
					if (is_supported(backend))
					{
						return backend;
					}
				}
			}

			if (s_backend.value() != RHIBackend::Undefined && is_supported(s_backend))
			{
				return s_backend.value();
			}

			RHIBackend fallback = Platform::System::instance()->graphics_backend();

			if (is_supported(fallback))
			{
				return fallback;
			}

			return RHIBackend::Undefined;
		}();

		return backend;
	}

	RHI* RHI::instance()
	{
		return instance(backend());
	}

	RHI* RHI::instance(RHIBackend backend)
	{
		auto it = backends().find(backend);

		if (it != backends().end())
		{
			auto& state = it->second;

			if (state.backend)
				return state.backend;

			state.backend = state.create();
		}

		return nullptr;
	}
} // namespace Trinex
