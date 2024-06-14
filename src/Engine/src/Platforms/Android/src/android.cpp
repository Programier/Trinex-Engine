#include <Core/base_engine.hpp>
#include <Core/config_manager.hpp>
#include <Core/definitions.hpp>
#include <Core/string_functions.hpp>
#include <android_native_app_glue.h>
#include <android_platform.hpp>
#include <jni.h>


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


namespace Engine::Platform
{
    AndroidPlatformInfo m_android_platform_info = {};

    ENGINE_EXPORT OperationSystemType system_type()
    {
        return OperationSystemType::Android;
    }

    ENGINE_EXPORT const char* system_name()
    {
        return "Android";
    }

    ENGINE_EXPORT Path find_root_directory()
    {
        String path = Strings::format("/sdcard/TrinexGames/{}/", ConfigManager::get_string("Engine::project_name"));
        return path;
    }

    ENGINE_EXPORT Vector<Pair<Path, Path>> hard_drives()
    {
        return {{"/", "/"}};
    }
}// namespace Engine::Platform
