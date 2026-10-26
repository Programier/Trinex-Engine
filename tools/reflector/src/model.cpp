#include <archive.hpp>
#include <cassert>
#include <iostream>
#include <model.hpp>
#include <reflector.hpp>
#include <string_view>

namespace Reflector
{
	static bool is_builtin_type(std::string_view name)
	{
		return name == "void" || name == "bool" || name == "char" || name == "signed" || name == "unsigned" || name == "short" ||
		       name == "int" || name == "long" || name == "float" || name == "double" || name == "wchar_t" || name == "char8_t" ||
		       name == "char16_t" || name == "char32_t" || name == "auto" || name.starts_with("std::");
	}

	static std::string qualified_name(const Object* object)
	{
		if (object == nullptr)
			return "";

		if (object->is_a(ObjectKind::Module) || object->owner == nullptr)
			return {};

		std::string result = qualified_name(object->owner);

		if (!result.empty())
			result += "::";

		result += object->name;
		return result;
	}

	static const Module* module_of(const Object* object)
	{
		while (object && !object->is_a(ObjectKind::Module)) object = object->owner;
		return static_cast<const Module*>(object);
	}

	static const char* object_kind_name(ObjectKind kind)
	{
		switch (kind)
		{
			case ObjectKind::Object: return "object";
			case ObjectKind::Enum: return "enum";
			case ObjectKind::Scope: return "scope";
			case ObjectKind::Namespace: return "namespace";
			case ObjectKind::Class: return "class";
			case ObjectKind::Module: return "module";
			case ObjectKind::Function: return "function";
			case ObjectKind::Property: return "property";
		}
		return "unknown";
	}

	static void link_error(const Object* context, std::string_view message)
	{
		const Module* module = module_of(context);
		std::cerr << "Error";

		if (module && !module->name.empty())
			std::cerr << " [" << module->name;

		if (context->line != 0)
			std::cerr << (module && !module->name.empty() ? ":" : " [") << context->line;

		if ((module && !module->name.empty()) || context->line != 0)
			std::cerr << ']';

		std::cerr << ": " << message << '\n';

		const std::string name = qualified_name(context);
		std::cerr << "  required by " << object_kind_name(context->kind());

		if (!name.empty())
			std::cerr << " '" << name << '\'';

		std::cerr << '\n';
	}

	static bool link_type(const Object* context, const TypeInfo& type, const TypeIndex& type_index,
	                      ObjectKind required_kind = ObjectKind::Object)
	{
		if (context == nullptr)
			return false;

		bool result = true;

		for (const TypeInfo::TemplateArgument& argument : type.templates)
		{
			if (argument.flags & TypeInfo::TemplateArgument::Type)
				result = link_type(context, argument.type, type_index) && result;
		}

		if (type.name.empty() || is_builtin_type(type.name))
			return result;

		const TypeIndex::Entry* object = type_index.resolve(context, type.name);

		if (!object)
		{
			link_error(context, "unknown reflected type '" + type.name + "'");
			return false;
		}

		if (required_kind != ObjectKind::Object && object->kind != required_kind)
		{
			link_error(context, "reflected type '" + type.name + "' is not a " + object_kind_name(required_kind) + " (found " +
			                            object_kind_name(object->kind) + ")");
			return false;
		}

		return result;
	}

	trinex_implement_reflector_type(Enum, values);
	trinex_implement_reflector_type(Namespace);
	trinex_implement_reflector_type(Scope, objects);
	trinex_implement_reflector_type(Class, bases, flags, body_line);
	trinex_implement_reflector_type(Module);
	trinex_implement_reflector_type(Function, type, args, flags);
	trinex_implement_reflector_type(Property, type, value, flags);

	void serialize(Archive& ar, Object*& object)
	{
		ObjectKind kind = object ? object->kind() : ObjectKind::Object;
		ar(kind);

		if (ar.reader())
		{
			assert(object == nullptr);
			object        = Object::create(kind);
			object->owner = ar.context();
		}

		object->serialize(ar);
	}

	bool TypeIndex::index_module(const Module* module)
	{
		if (module == nullptr)
			return false;

		bool result = true;
		auto& names = m_modules[module->name];

		auto visit = [&](auto&& self, const Object* object) -> void {
			if (object == nullptr)
				return;


			if (object->is_a(ObjectKind::Class) || object->is_a(ObjectKind::Enum))
			{
				const std::string name = qualified_name(object);
				if (auto [_, inserted] = m_types.emplace(name, Entry{object->kind(), module->name}); !inserted)
				{
					std::cerr << "Error: duplicate reflected type '" << name << "'\n";
					result = false;
				}
				else
				{
					names.push_back(name);
				}
			}

			if (const auto* scope = dynamic_cast<const Scope*>(object))
			{
				for (const Object* child : scope->objects)
				{
					self(self, child);
				}
			}
		};

		for (const Object* object : module->objects)
		{
			visit(visit, object);
		}

		return result;
	}

	void TypeIndex::remove_module(std::string_view name)
	{
		const auto it = m_modules.find(std::string(name));
		if (it == m_modules.end())
			return;

		for (const std::string& type_name : it->second)
		{
			m_types.erase(type_name);
		}

		m_modules.erase(it);
	}

	const TypeIndex::Entry* TypeIndex::resolve(const Object* context, std::string_view name) const
	{
		if (context == nullptr)
			return nullptr;

		if (name.starts_with("::"))
			name.remove_prefix(2);

		for (const Object* scope = context->owner; scope && !scope->is_a(ObjectKind::Module); scope = scope->owner)
		{
			const std::string prefix = qualified_name(scope);

			if (!prefix.empty())
			{
				if (const auto it = m_types.find(prefix + "::" + std::string(name)); it != m_types.end())
					return &it->second;
			}
		}

		if (const auto it = m_types.find(std::string(name)); it != m_types.end())
			return &it->second;
		return nullptr;
	}

	void TypeIndex::serialize(Archive& ar)
	{
		std::vector<std::string> names;
		std::vector<Entry> entries;

		if (ar.writer())
		{
			names.reserve(m_types.size());
			entries.reserve(m_types.size());
			for (const auto& [name, entry] : m_types)
			{
				names.push_back(name);
				entries.push_back(entry);
			}
		}

		ar(names, entries);

		if (!ar.reader())
			return;

		m_types.clear();
		m_modules.clear();
		m_types.reserve(names.size());

		for (std::size_t index = 0; index < names.size(); ++index)
		{
			m_modules[entries[index].module].push_back(names[index]);
			m_types.emplace(std::move(names[index]), std::move(entries[index]));
		}
	}

	Object* Object::create(ObjectKind kind)
	{
		switch (kind)
		{
			case ObjectKind::Object: return new Object();
			case ObjectKind::Enum: return new Enum();
			case ObjectKind::Scope: return new Scope();
			case ObjectKind::Namespace: return new Namespace();
			case ObjectKind::Class: return new Class();
			case ObjectKind::Module: return new Module();
			case ObjectKind::Function: return new Function();
			case ObjectKind::Property: return new Property();
			default: return nullptr;
		}
	}

	Object* Object::find(std::string_view, ObjectKind)
	{
		return nullptr;
	}

	ObjectKind Object::kind() const
	{
		return ObjectKind::Object;
	}

	bool Object::link(const TypeIndex&)
	{
		return true;
	}

	bool Object::is_a(ObjectKind kind) const
	{
		return kind == ObjectKind::Object;
	}

	void Object::serialize(Archive& ar)
	{
		ar(name, line, access, metadata);
	}

	Object::~Object() {}

	Object* Scope::find(std::string_view name, ObjectKind kind)
	{
		if (name.starts_with("::"))
		{
			Object* root = this;
			while (root->owner) root = root->owner;
			return root->find(name.substr(2), kind);
		}
		const auto separator  = name.find("::");
		const auto local_name = name.substr(0, separator);

		for (const auto& object : objects)
		{
			if (!object || (object->kind() != ObjectKind::Module && object->name != local_name))
				continue;

			if (separator != std::string_view::npos)
			{
				if (auto* found = object->find(name.substr(separator + 2), kind))
					return found;
			}
			else if (object->is_a(kind))
			{
				return object;
			}
		}

		return nullptr;
	}

	bool Scope::link(const TypeIndex& type_index)
	{
		for (Object* child : objects)
		{
			if (!child->link(type_index))
				return false;
		}

		return true;
	}

	Scope::~Scope()
	{
		for (Object* object : objects)
		{
			delete object;
		}
	}

	bool Function::link(const TypeIndex& type_index)
	{
		bool result = Super::link(type_index) && link_type(this, type, type_index);
		for (const Argument& argument : args) result = link_type(this, argument.type, type_index) && result;
		return result;
	}

	bool Property::link(const TypeIndex& type_index)
	{
		const bool result = Super::link(type_index);
		return link_type(this, type, type_index) && result;
	}
}// namespace Reflector
