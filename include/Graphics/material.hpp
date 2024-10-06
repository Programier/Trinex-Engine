#pragma once
#include <Core/object.hpp>

namespace Engine
{
	namespace ShaderCompiler
	{
		class Compiler;
		struct ShaderSource;
	}// namespace ShaderCompiler

	namespace MaterialParameters
	{
		class Parameter;
	}

	class SceneComponent;
	class Pipeline;
	struct ShaderDefinition;

	class ENGINE_EXPORT MaterialInterface : public Object
	{
		declare_class(MaterialInterface, Object);

	public:
		using Parameter = MaterialParameters::Parameter;

	protected:
		Vector<Parameter*> m_parameters;
		bool register_child_internal(Object* child, const Name& name);
		bool register_child(Object* child) override;
		bool unregister_child(Object* child) override;
		bool rename_child_object(Object* object, StringView new_name) override;

	public:
		Object* find_child_object(StringView name, bool recursive = true) const override;
		Parameter* find_parameter(const Name& name) const;
		MaterialInterface& remove_parameter(const Name& name);
		MaterialInterface& clear_parameters();
		const Vector<Parameter*>& parameters() const;

		template<typename T>
		T* find_parameter(const Name& name) const
		{
			return instance_cast<T>(find_parameter(name));
		}

		virtual MaterialInterface* parent() const;
		virtual class Material* material();
		virtual bool apply(SceneComponent* component = nullptr);

		bool archive_process(Archive& archive) override;

		~MaterialInterface();
	};

	class ENGINE_EXPORT Material : public MaterialInterface
	{
		declare_class(Material, MaterialInterface);

	public:
		Pipeline* pipeline;
		Vector<ShaderDefinition> compile_definitions;

		Material();

		Material& preload() override;
		Material& postload() override;
		class Material* material() override;
		Material& apply_changes() override;
		bool apply(SceneComponent* component = nullptr) override;
		bool apply(MaterialInterface* head, SceneComponent* component = nullptr);

		virtual bool compile(ShaderCompiler::Compiler* compiler = nullptr, MessageList* errors = nullptr);
		virtual bool shader_source(String& out_source) = 0;

		bool submit_compiled_source(const ShaderCompiler::ShaderSource& source, MessageList& errors);
		bool archive_process(Archive& archive) override;
		~Material();
	};

	class ENGINE_EXPORT MaterialInstance : public MaterialInterface
	{
		declare_class(MaterialInstance, MaterialInterface);

	public:
		MaterialInterface* parent_material = nullptr;

		class Material* material() override;
		MaterialInterface* parent() const override;
		bool apply(SceneComponent* component = nullptr) override;
		bool archive_process(Archive& archive) override;
		~MaterialInstance();
	};
}// namespace Engine
