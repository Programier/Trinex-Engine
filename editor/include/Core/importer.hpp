#pragma once
#include <Core/transform.hpp>

namespace Engine
{
	class Package;
	class Path;
	class World;
}// namespace Engine

namespace Engine::Importer
{
	void import_scene(Package* package, const Path& file, World* world = nullptr, const Transform& transform = {});
}// namespace Engine::Importer
