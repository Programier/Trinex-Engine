#pragma once
#include <model.hpp>
#include <vector>

namespace Reflector
{
	class Object;
	class Scope;
	class Module;
	class Namespace;
	class Struct;
	class Class;
	class Enum;
	class Function;
	class Property;
	struct TypeInfo;

	class Generator
	{
	public:
		class Plugin
		{
		public:
			virtual void begin(std::ostream& out, const Object* root);
			virtual void end(std::ostream& out, const Object* root);

			virtual void generate_object(std::ostream& out, const Object* object);
			virtual void generate_scope(std::ostream& out, const Scope* scope);
			virtual void generate_namespace(std::ostream& out, const Namespace* ns);
			virtual void generate_struct(std::ostream& out, const Struct* type);
			virtual void generate_class(std::ostream& out, const Class* type);
			virtual void generate_enum(std::ostream& out, const Enum* type);
			virtual void generate_function(std::ostream& out, const Function* function);
			virtual void generate_property(std::ostream& out, const Property* property);
			virtual ~Plugin();
		};

	private:
		std::vector<Diagnostic> m_diagnostics;

	public:
		bool generate(std::ostream& out, const Object* object);
		bool generate(std::ostream& out, const Object* object, std::initializer_list<Plugin*> plugins);
		const std::vector<Diagnostic>& diagnostics() const noexcept;
	};
}// namespace Reflector
