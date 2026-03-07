#pragma once
#include <Core/transform.hpp>

namespace Trinex
{
	class Package;
	class Path;
	class World;
}// namespace Trinex

namespace Trinex::Importer
{
	void import_scene(Package* package, const Path& file, World* world = nullptr, const Transform& transform = {});
}// namespace Trinex::Importer
