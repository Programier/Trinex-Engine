#pragma once
#include <cstddef>
struct ImGuiContext;

namespace Engine
{
	class RenderSurface;
	class Texture2D;
}// namespace Engine

struct ImGuiTrinexTextureId {
	Engine::Texture2D* texture;
	Engine::RenderSurface* surface;

	inline constexpr ImGuiTrinexTextureId() : texture(nullptr), surface(nullptr) {}
	inline consteval ImGuiTrinexTextureId(decltype(NULL)) : ImGuiTrinexTextureId() {}
	inline consteval ImGuiTrinexTextureId(decltype(nullptr)) : ImGuiTrinexTextureId() {}
	inline ImGuiTrinexTextureId(Engine::Texture2D* texture) : texture(texture), surface(nullptr) {}
	inline ImGuiTrinexTextureId(Engine::RenderSurface* surface) : texture(nullptr), surface(surface) {}
	inline bool operator==(const ImGuiTrinexTextureId& other) const { return other.texture == texture; }
	inline bool operator!=(const ImGuiTrinexTextureId& other) const { return other.texture != texture; }
	inline const void* id() const { return texture ? static_cast<const void*>(texture) : static_cast<const void*>(surface); }
	inline operator bool() const { return id() != nullptr; }
	inline operator const void*() const { return id(); }
};

ImGuiContext*& get_current_imgui_context();

#define ImTextureID ImGuiTrinexTextureId
#define GImGui get_current_imgui_context()
#define IMGUI_DEFINE_MATH_OPERATORS
