#pragma once
#include <Core/transform.hpp>

namespace Engine
{
	class Package;
	class Path;
}// namespace Engine

namespace Engine::Importer
{
	void import_resource(Package* package, const Path& file, const Transform& transform = {});
}
