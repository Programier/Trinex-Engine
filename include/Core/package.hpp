#pragma once

#include <Core/etl/object_tree_node.hpp>
#include <Core/deprecated/object.hpp>

namespace Trinex
{

	class trinex_api Package : public ObjectTreeNode<DeprecatedObject, DeprecatedObject>
	{
		trinex_class(Package, DeprecatedObject);

		bool can_add_object(DeprecatedObject* object) const;

	protected:
		DeprecatedObject* register_child(DeprecatedObject* object, u32& index) override;

	public:
		Package();

		trinex_non_copyable(Package);
		trinex_non_moveable(Package);

		bool add_object(DeprecatedObject* object);
		Package& remove_object(DeprecatedObject* object);

		const Array<DeprecatedObject*>& objects() const;
		bool contains_object(const DeprecatedObject* object) const;
		bool contains_object(const StringView& name) const;
		Package* find_package(StringView child, bool create = false);
		bool save(Stream* stream = nullptr, ArchiveFlags flags = {}) override;
		friend class DeprecatedObject;
	};
} // namespace Trinex
