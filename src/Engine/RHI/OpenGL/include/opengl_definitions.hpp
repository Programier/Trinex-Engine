#pragma once
#include <Core/definitions.hpp>

#define OPENGL_EXTENDS_FROM_NOAPI 0

#if PLATFORM_ANDROID || FORCE_USE_OPENGL_ES
#define USING_OPENGL_ES 1
#define USING_OPENGL_CORE 0
#else
#define USING_OPENGL_ES 0
#define USING_OPENGL_CORE 1
#endif
