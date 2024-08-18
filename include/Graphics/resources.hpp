#pragma once

#include <Core/export.hpp>

namespace Engine
{
	class BasicMesh;
	class Texture;
	class Drawable;

	namespace Resources
	{
		ENGINE_EXPORT extern Vector<BasicMesh*> meshes;
		ENGINE_EXPORT extern Vector<Texture*> textures;
		ENGINE_EXPORT extern Vector<Drawable*> drawables;
	}// namespace Resources

}// namespace Engine
