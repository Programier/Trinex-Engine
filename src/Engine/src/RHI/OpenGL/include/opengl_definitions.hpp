#pragma once
#include <Core/definitions.hpp>

#if PLATFORM_ANDROID
#define USING_OPENGL_ES 1
#define USING_OPENGL_CORE 0
#else
#define USING_OPENGL_ES 0
#define USING_OPENGL_CORE 1
#endif
