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
	void import_resource(Package* package, const Path& file, const Transform& transform = {});
	void import_scene(World* world, Package* package, const Path& file, const Transform& transform = {});
}// namespace Engine::Importer
