#pragma once
#include <cstddef>
struct ImGuiContext;

namespace Trinex
{
	class RHITexture;
	class RHISampler;
}// namespace Trinex

struct ImGuiTrinexTextureId {
	Trinex::RHITexture* texture;
	Trinex::RHISampler* sampler;

	inline constexpr ImGuiTrinexTextureId(Trinex::RHITexture* texture = nullptr, Trinex::RHISampler* sampler = nullptr)
	    : texture(texture), sampler(sampler)
	{}

	inline consteval ImGuiTrinexTextureId(decltype(NULL)) : ImGuiTrinexTextureId() {}
	inline consteval ImGuiTrinexTextureId(decltype(nullptr)) : ImGuiTrinexTextureId() {}

	inline bool operator==(const ImGuiTrinexTextureId& other) const { return other.texture == texture; }
	inline bool operator!=(const ImGuiTrinexTextureId& other) const { return other.texture != texture; }
	inline const void* id() const { return texture; }
	inline operator bool() const { return id() != nullptr; }
	inline operator const void*() const { return id(); }
};

ImGuiContext*& get_current_imgui_context();

#define ImTextureID ImGuiTrinexTextureId
#define GImGui get_current_imgui_context()
#define IMGUI_DEFINE_MATH_OPERATORS
