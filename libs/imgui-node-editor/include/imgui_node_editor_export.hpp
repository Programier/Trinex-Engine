#pragma once

#ifndef IMGUI_NODE_EDITOR_EXPORT

#ifdef _WIN32
#if defined( IMGUI_NODE_EDITOR_EXPORT_ENABLE ) || defined( ENABLE_ENGINE_EXPORTS )
#define IMGUI_NODE_EDITOR_EXPORT __declspec(dllexport)
#else // !BUILDING_DLL
#define IMGUI_NODE_EDITOR_EXPORT __declspec(dllimport)
#endif // BUILDING_DLL
#else
#if defined ( IMGUI_NODE_EDITOR_EXPORT_ENABLE ) || defined( ENABLE_ENGINE_EXPORTS )
#define IMGUI_NODE_EDITOR_EXPORT __attribute__((visibility("default")))
#else
#define IMGUI_NODE_EDITOR_EXPORT
#endif
#endif // _WIN32

#endif
