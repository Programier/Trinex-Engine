#pragma once
#include <cstddef>
struct ImGuiContext;

namespace Trinex
{
	class RHITexture;
	class RHISampler;
	class RHIContext;
}// namespace Trinex

struct ImDrawList;
struct ImDrawCmd;
struct ImDrawData;

struct ImGuiTrinexTextureId {
	Trinex::RHITexture* texture;
	Trinex::RHISampler* sampler;

	inline constexpr ImGuiTrinexTextureId(Trinex::RHITexture* texture = nullptr, Trinex::RHISampler* sampler = nullptr)
	    : texture(texture), sampler(sampler)
	{}

	inline consteval ImGuiTrinexTextureId(int) : ImGuiTrinexTextureId() {}

	inline bool operator==(const ImGuiTrinexTextureId& other) const { return other.texture == texture; }
	inline bool operator!=(const ImGuiTrinexTextureId& other) const { return other.texture != texture; }
	inline const void* id() const { return texture; }
	inline operator bool() const { return id() != nullptr; }
	inline operator const void*() const { return id(); }
};

struct ImDrawCallbackArgs {
	Trinex::RHIContext* ctx;
	Trinex::RHITexture* color;
	
	const ImDrawData* data;
	const ImDrawList* list;
	const ImDrawCmd* cmd;
};

using ImDrawCallbackDecl = void (*)(const ImDrawCallbackArgs& args);

ImGuiContext*& get_current_imgui_context();

#define ImDrawCallback ImDrawCallbackDecl
#define ImDrawCallbackFunc +[](const ImDrawCallbackArgs& args)
#define ImTextureID ImGuiTrinexTextureId
#define GImGui get_current_imgui_context()
#define IMGUI_DEFINE_MATH_OPERATORS
