#include <generator.hpp>
#include <ostream>
#include <stream.hpp>

namespace Reflector
{
	void Generator::Plugin::begin(std::ostream& out, const Object* root) {}
	void Generator::Plugin::end(std::ostream& out, const Object* root) {}
	void Generator::Plugin::generate_object(std::ostream& out, const Object* object) {}
	void Generator::Plugin::generate_scope(std::ostream& out, const Scope* scope) {}
	void Generator::Plugin::generate_namespace(std::ostream& out, const Namespace* ns) {}
	void Generator::Plugin::generate_struct(std::ostream& out, const Struct* type) {}
	void Generator::Plugin::generate_class(std::ostream& out, const Class* type) {}
	void Generator::Plugin::generate_enum(std::ostream& out, const Enum* type) {}
	void Generator::Plugin::generate_function(std::ostream& out, const Function* function) {}
	void Generator::Plugin::generate_property(std::ostream& out, const Property* property) {}

	Generator::Plugin::~Plugin() {}

	bool Generator::generate(std::ostream& out, const Object* object)
	{
		return generate(out, object, {});
	}

	bool Generator::generate(std::ostream& out, const Object* object, std::initializer_list<Plugin*> plugins)
	{
		auto stream        = code_stream(out);
		std::ostream& code = *stream.get();
		return true;
	}

	const std::vector<Diagnostic>& Generator::diagnostics() const noexcept
	{
		return m_diagnostics;
	}
}// namespace Reflector
