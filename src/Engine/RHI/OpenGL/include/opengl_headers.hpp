#pragma once

#ifdef _WIN32
#include <GL/glew.h>
////////////////////////
#include <GL/gl.h>

#if !defined(GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS) && defined(GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT)
#define GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT
#endif

#if !defined(GL_DEPTH_CLAMP_EXT) && defined(GL_DEPTH_CLAMP)
#define GL_DEPTH_CLAMP_EXT GL_DEPTH_CLAMP
#endif

#else
#include <GLES3/gl32.h>
////////////////////////
#include <GLES2/gl2ext.h>

#if !defined(GL_DEPTH_CLAMP_EXT) && defined (GL_DEPTH_CLAMP)
#define GL_DEPTH_CLAMP_EXT GL_DEPTH_CLAMP
#endif

#endif

#include <Core/rhi_initializers.hpp>
