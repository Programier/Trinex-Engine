#include <Core/base_engine.hpp>
#include <Core/config_manager.hpp>
#include <Core/definitions.hpp>
#include <Core/filesystem/path.hpp>
#include <Core/logger.hpp>
#include <Core/string_functions.hpp>
#include <android_native_app_glue.h>
#include <android_platform.hpp>
#include <jni.h>
#include <unistd.h>


static Engine::String to_string(JNIEnv* env, jstring str)
{
	const char* c_str = env->GetStringUTFChars(str, nullptr);
	Engine::String result(c_str);
	env->ReleaseStringUTFChars(str, c_str);
	return result;
}


extern "C" JNIEXPORT void JNICALL Java_com_TrinexEngine_TrinexActivity_initializePlatformInfo(
        JNIEnv* env, jobject thiz, jstring app_package_name, jstring device_manufacturer, jstring device_model,
        jstring device_build_number, jstring system_version, jstring system_language, jstring cache_dir, jstring executable_path,
        jstring libraries_path, jint screen_width, jint screen_height)
{
	using namespace Engine::Platform;
	m_android_platform_info.app_package_name    = to_string(env, app_package_name);
	m_android_platform_info.device_manufacturer = to_string(env, device_manufacturer);
	m_android_platform_info.device_model        = to_string(env, device_model);
	m_android_platform_info.device_build_number = to_string(env, device_build_number);
	m_android_platform_info.system_version      = to_string(env, system_version);
	m_android_platform_info.system_language     = to_string(env, system_language);
	m_android_platform_info.cache_dir           = to_string(env, cache_dir);
	m_android_platform_info.executable_path     = to_string(env, executable_path);
	m_android_platform_info.libraries_path      = to_string(env, libraries_path);
	m_android_platform_info.screen_width        = screen_width;
	m_android_platform_info.screen_height       = screen_height;
}

extern "C" JNIEXPORT void JNICALL Java_com_TrinexEngine_TrinexActivity_updateOrientation(JNIEnv* env, jobject thiz,
                                                                                         jint orientation)
{
	using namespace Engine::Platform;

	m_android_platform_info.orientation            = static_cast<Engine::Orientation>(orientation);
	m_android_platform_info.is_orientation_updated = true;
}


namespace Engine::Platform
{
	AndroidPlatformInfo m_android_platform_info = {};
	static android_app* m_application           = nullptr;

	ENGINE_EXPORT OperationSystemType system_type()
	{
		return OperationSystemType::Android;
	}

	ENGINE_EXPORT const char* system_name()
	{
		return "Android";
	}

	ENGINE_EXPORT Path find_exec_directory()
	{
		return Path("/sdcard/Android/obb") / m_android_platform_info.app_package_name;
	}

	ENGINE_EXPORT void bind_platform_mount_points()
	{}


	ENGINE_EXPORT Vector<Pair<Path, Path>> hard_drives()
	{
		return {{"/", "/"}};
	}

	ENGINE_EXPORT size_t monitors_count()
	{
		return 1;
	}

	ENGINE_EXPORT MonitorInfo monitor_info(Index monitor_index)
	{
		MonitorInfo info;
		info.pos  = {0, 0};
		info.size = {m_android_platform_info.screen_width, m_android_platform_info.screen_height};
		//info.dpi = m_android_platform_info.
		return info;
	}

	void initialize_android_application(struct android_app* app)
	{
		m_application = app;

		initialize_android_events_callbacks(app);

		while (m_android_platform_info.is_inited == false)
		{
			Platform::EventSystem::wait_for_events([](const Event& e, void*) {}, nullptr);
		}

		Path exec_dir = find_exec_directory();
		chdir(exec_dir.c_str());
	}

	android_app* android_application()
	{
		return m_application;
	}
}// namespace Engine::Platform
