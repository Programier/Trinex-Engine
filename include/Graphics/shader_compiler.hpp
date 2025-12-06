#pragma once
#include <Core/etl/vector.hpp>
#include <Core/object.hpp>
#include <RHI/structures.hpp>

namespace Engine
{
	class Logger;
	class Material;
	class Pipeline;
	class Path;
	class RenderPass;

	class ShaderCompilationEnvironment
	{
	public:
		virtual ShaderCompilationEnvironment& add_module(const char* module) = 0;
		virtual ShaderCompilationEnvironment& add_source(const char* source) = 0;

		virtual size_t modules_count() const = 0;
		virtual size_t sources_count() const = 0;

		virtual const char* module(size_t index) const = 0;
		virtual const char* source(size_t index) const = 0;
	};

	class ShaderCompilationResult
	{
	public:
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
			Vector<RHIVertexAttribute> vertex_attributes;
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
		template<template<class T> typename AllocatorType = Allocator>
		class Environment : public ShaderCompilationEnvironment
		{
		private:
			Vector<const char*, AllocatorType<const char*>> m_modules;
			Vector<const char*, AllocatorType<const char*>> m_sources;

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

			size_t modules_count() const override { return m_modules.size(); }
			size_t sources_count() const override { return m_sources.size(); }

			const char* module(size_t index) const override { return m_modules[index]; }
			const char* source(size_t index) const override { return m_sources[index]; }
		};

		class StackEnvironment : public Environment<StackAllocator>
		{
		private:
			StackByteAllocator::Mark m_stack_mark;
		};

	public:
		static ShaderCompiler* instance(const StringView& api_name = "");
		virtual bool compile(const ShaderCompilationEnvironment* env, ShaderCompilationResult& result) = 0;
	};

}// namespace Engine
