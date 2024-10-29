#pragma once
#include <Core/engine_types.hpp>
#include <Core/export.hpp>
#include <initializer_list>

namespace Engine
{
	using ControllerCallback = Function<void()>;

	class ENGINE_EXPORT LoadingControllerBase
	{
	private:
		void* m_list;
		const char* m_name = nullptr;

	protected:
		LoadingControllerBase(void* list, const char* name);

		static bool is_triggered(BitMask type);
		static void mark_triggered(BitMask type);

	public:
		static void exec_all_if_already_triggered();

		LoadingControllerBase& push(const ControllerCallback& callback, const String& name = "",
		                            const std::initializer_list<String>& require_initializers = {});
		LoadingControllerBase& require(const String& name);
		LoadingControllerBase& execute();
		virtual Identifier id() const = 0;

		virtual ~LoadingControllerBase();
	};

#define IMPLEMENT_CONTROLLER(ControllerName)                                                                                     \
	class ENGINE_EXPORT ControllerName : public LoadingControllerBase                                                            \
	{                                                                                                                            \
	public:                                                                                                                      \
		ControllerName();                                                                                                        \
		ControllerName(const ControllerCallback& callback, const String& name = "",                                              \
		               const std::initializer_list<String>& require_initializers = {});                                          \
		ControllerName& push(const ControllerCallback& callback, const String& name = "",                                        \
		                     const std::initializer_list<String>& require_initializers = {});                                    \
		ControllerName& require(const String& name);                                                                             \
		ControllerName& execute();                                                                                               \
		Identifier id() const override;                                                                                          \
		static bool is_triggered();                                                                                              \
	}

	IMPLEMENT_CONTROLLER(PreInitializeController);
	IMPLEMENT_CONTROLLER(InitializeController);

	IMPLEMENT_CONTROLLER(PostDestroyController);
	IMPLEMENT_CONTROLLER(DestroyController);

	IMPLEMENT_CONTROLLER(StartupResourcesInitializeController);
	IMPLEMENT_CONTROLLER(ConfigsInitializeController);

	IMPLEMENT_CONTROLLER(ReflectionInitializeController);
	IMPLEMENT_CONTROLLER(ReflectionPostInitializeController);

	IMPLEMENT_CONTROLLER(ScriptAddonsInitializeController);
#undef IMPLEMENT_CONTROLLER
}// namespace Engine
