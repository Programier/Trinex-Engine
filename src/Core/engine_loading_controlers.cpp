#include <Core/base_engine.hpp>
#include <Core/demangle.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/engine_types.hpp>
#include <Core/etl/list.hpp>
#include <Core/flags.hpp>
#include <Core/logger.hpp>
#include <Core/reflection/object.hpp>
#include <Core/threading.hpp>

namespace Engine
{
	enum class ControllerType : BitMask
	{
		InitializerMask = BIT(63),

		PreInit           = BIT(0) | InitializerMask,
		Init              = BIT(1) | InitializerMask,
		Destroy           = BIT(2),
		PostDestroy       = BIT(3),
		ReflectionInit    = BIT(4) | InitializerMask,
		ResourcesInit     = BIT(5) | InitializerMask,
		ConfigsInitialize = BIT(6) | InitializerMask,
	};

	struct CallbackEntry {
		ControllerCallback function;
		Vector<String> require_initializers;
	};

	using CallbacksList = TreeMap<String, List<CallbackEntry>>;

	template<typename T>
	static CallbacksList* callbacks_list()
	{
		static CallbacksList list;
		return &list;
	}

	static Flags<ControllerType, BitMask> m_triggered;

#define list_of(ptr) (*reinterpret_cast<CallbacksList*>(ptr))

	LoadingControllerBase::LoadingControllerBase(void* list, const char* name) : m_list(list), m_name(name) {}

	void LoadingControllerBase::exec_all_if_already_triggered()
	{
		if (PreInitializeController::is_triggered())
		{
			PreInitializeController().execute();
		}

		if (ConfigsInitializeController::is_triggered())
		{
			ConfigsInitializeController().execute();
		}

		if (ReflectionInitializeController::is_triggered())
		{
			ReflectionInitializeController().execute();
		}

		if (StartupResourcesInitializeController::is_triggered())
		{
			StartupResourcesInitializeController().execute();
		}

		if (InitializeController::is_triggered())
		{
			InitializeController().execute();
		}
	}

	LoadingControllerBase& LoadingControllerBase::push(const ControllerCallback& callback, const String& name,
	                                                   const std::initializer_list<String>& require_initializers)
	{
		CallbackEntry entry;
		entry.function             = callback;
		entry.require_initializers = require_initializers;
		list_of(m_list)[name].push_back(entry);

		if (is_triggered(id()) && (id() & static_cast<BitMask>(ControllerType::InitializerMask)) ==
		                                  static_cast<BitMask>(ControllerType::InitializerMask))
		{
			static bool submit_command = true;

			if (submit_command)
			{
				submit_command = false;

				logic_thread()->add_task(Task(Task::High, []() {
					submit_command = true;
					exec_all_if_already_triggered();
				}));
			}
		}

		return *this;
	}

	LoadingControllerBase& LoadingControllerBase::require(const String& name)
	{
		auto it = list_of(m_list).find(name);
		if (it == list_of(m_list).end())
			return *this;

		List<CallbackEntry>& list = it->second;

		while (!list.empty())
		{
			CallbackEntry entry = list.front();
			list.pop_front();

			for (auto& initializer : entry.require_initializers)
			{
				require(initializer);
			}

			entry.function();
		}

		return *this;
	}


	LoadingControllerBase& LoadingControllerBase::execute()
	{
		info_log(m_name, "Executing command list!");
		mark_triggered(id());

		while (!list_of(m_list).empty())
		{
			auto id = list_of(m_list).begin();
			require(id->first);
			list_of(m_list).erase(id);
		}

		return *this;
	}

	bool LoadingControllerBase::is_triggered(BitMask type)
	{
		return (m_triggered & static_cast<ControllerType>(type)) == type;
	}

	void LoadingControllerBase::mark_triggered(BitMask type)
	{
		m_triggered |= static_cast<ControllerType>(type);
	}

	LoadingControllerBase::~LoadingControllerBase() {}

#define IMPLEMENT_CONTROLLER(ControllerName, type)                                                                               \
	ControllerName::ControllerName() : LoadingControllerBase(callbacks_list<ControllerName>(), #ControllerName) {}               \
                                                                                                                                 \
	ControllerName::ControllerName(const ControllerCallback& callback, const String& name,                                       \
	                               const std::initializer_list<String>& require_initializers)                                    \
	    : ControllerName()                                                                                                       \
	{                                                                                                                            \
		push(callback, name, require_initializers);                                                                              \
	}                                                                                                                            \
	ControllerName& ControllerName::push(const ControllerCallback& callback, const String& name,                                 \
	                                     const std::initializer_list<String>& require_initializers)                              \
	{                                                                                                                            \
		LoadingControllerBase::push(callback, name, require_initializers);                                                       \
		return *this;                                                                                                            \
	}                                                                                                                            \
	ControllerName& ControllerName::require(const String& name)                                                                  \
	{                                                                                                                            \
		LoadingControllerBase::require(name);                                                                                    \
		return *this;                                                                                                            \
	}                                                                                                                            \
	bool ControllerName::is_triggered()                                                                                          \
	{                                                                                                                            \
		return LoadingControllerBase::is_triggered(static_cast<BitMask>(ControllerType::type));                                  \
	}                                                                                                                            \
	Identifier ControllerName::id() const                                                                                        \
	{                                                                                                                            \
		return static_cast<BitMask>(ControllerType::type);                                                                       \
	}

	IMPLEMENT_CONTROLLER(PreInitializeController, PreInit);
	IMPLEMENT_CONTROLLER(InitializeController, Init);

	IMPLEMENT_CONTROLLER(DestroyController, Destroy);
	IMPLEMENT_CONTROLLER(PostDestroyController, PostDestroy);

	IMPLEMENT_CONTROLLER(ReflectionInitializeController, ReflectionInit);
	ReflectionInitializeController& ReflectionInitializeController::execute()
	{
		LoadingControllerBase::execute();
		Refl::Object::static_initialize();
		return *this;
	}

	IMPLEMENT_CONTROLLER(StartupResourcesInitializeController, ResourcesInit);
	IMPLEMENT_CONTROLLER(ConfigsInitializeController, ConfigsInitialize);
}// namespace Engine
