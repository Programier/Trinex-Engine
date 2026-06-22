#pragma once
#include <Core/etl/function.hpp>
#include <Core/etl/vector.hpp>
#include <Core/object.hpp>
#include <RHI/structures.hpp>

namespace Trinex
{
	class Logger;
	class Material;
	class Pipeline;
	class Path;
	class RenderPass;

	class ShaderCompilationEnvironment
	{
	public:
		virtual ShaderCompilationEnvironment& add_module(const char* module)                 = 0;
		virtual ShaderCompilationEnvironment& add_source(const char* source)                 = 0;
		virtual ShaderCompilationEnvironment& add_specialization_arg(const char* expression) = 0;

		virtual usize modules_count() const             = 0;
		virtual usize sources_count() const             = 0;
		virtual usize specialization_args_count() const = 0;

		virtual const char* module(usize index) const             = 0;
		virtual const char* source(usize index) const             = 0;
		virtual const char* specialization_arg(usize index) const = 0;
	};

	class ShaderCompilationResult
	{
	public:
		Name permutation;

		struct Shaders {
			Buffer vertex;
			Buffer tessellation_control;
			Buffer tessellation;
			Buffer geometry;
			Buffer fragment;
			Buffer compute;

			// Mesh shaders support
			Buffer mesh;
			Buffer task;

			// Raytracing shaders support
			Buffer raygen;
			Buffer closest_hit;
			Buffer any_hit;
			Buffer miss;
			Buffer intersection;
		} shaders;

		struct Reflection {
			Vector<RHIInputAttribute> vertex_attributes;
			Vector<RHIShaderParameterInfo> parameters;
		} reflection;

	public:
		bool initialize_pipeline(class GraphicsPipeline* pipeline);
		bool initialize_pipeline(class ComputePipeline* pipeline);
	};

	class ENGINE_EXPORT ShaderCompiler : public Object
	{
		trinex_class(ShaderCompiler, Object);

	public:
		using CompileCallback = Function<bool(const ShaderCompilationResult&)>;

	public:
		template<template<class T> typename AllocatorType = Allocator>
		class Environment : public ShaderCompilationEnvironment
		{
		private:
			Vector<const char*, AllocatorType<const char*>> m_modules;
			Vector<const char*, AllocatorType<const char*>> m_sources;
			Vector<const char*, AllocatorType<const char*>> m_specialization_args;

		public:
			ShaderCompilationEnvironment& add_module(const char* module) override
			{
				m_modules.push_back(module);
				return *this;
			}

			ShaderCompilationEnvironment& add_source(const char* source) override
			{
				m_sources.push_back(source);
				return *this;
			}

			ShaderCompilationEnvironment& add_specialization_arg(const char* expression) override
			{
				m_specialization_args.push_back(expression);
				return *this;
			}

			usize modules_count() const override { return m_modules.size(); }
			usize sources_count() const override { return m_sources.size(); }
			usize specialization_args_count() const override { return m_specialization_args.size(); }

			const char* module(usize index) const override { return m_modules[index]; }
			const char* source(usize index) const override { return m_sources[index]; }
			const char* specialization_arg(usize index) const override { return m_specialization_args[index]; }
		};

		class StackEnvironment : public Environment<StackAllocator>
		{
		private:
			StackByteAllocator::Mark m_stack_mark;
		};

	public:
		ShaderCompiler();
		~ShaderCompiler();

		static ShaderCompiler* instance(const StringView& api_name = "");
		bool compile(const ShaderCompilationEnvironment* env, ShaderCompilationResult& result);
		virtual bool compile(const ShaderCompilationEnvironment* env, const CompileCallback& callback) = 0;
	};

}// namespace Trinex
