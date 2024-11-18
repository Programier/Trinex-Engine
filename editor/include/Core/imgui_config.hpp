struct ImGuiContext;

namespace Engine
{
	class Texture2D;
	class Sampler;
}// namespace Engine

struct ImGuiTrinexTextureId {
	Engine::Texture2D* texture;
	Engine::Sampler* sampler;

	constexpr ImGuiTrinexTextureId(Engine::Texture2D* texture = nullptr, Engine::Sampler* sampler = nullptr)
		: texture(texture), sampler(sampler)
	{}
	inline bool operator==(const ImGuiTrinexTextureId& other) const
	{
		return other.texture == texture && other.sampler == sampler;
	}
	inline bool operator!=(const ImGuiTrinexTextureId& other) const
	{
		return other.texture != texture || other.sampler != sampler;
	}
	inline void* id() const
	{
		return texture;
	}
	inline operator bool() const
	{
		return texture != nullptr;
	}

	inline operator const void*() const
	{
		return id();
	}
};

ImGuiContext*& get_current_imgui_context();

#define ImTextureID ImGuiTrinexTextureId
#define GImGui get_current_imgui_context()
#define IMGUI_DEFINE_MATH_OPERATORS
