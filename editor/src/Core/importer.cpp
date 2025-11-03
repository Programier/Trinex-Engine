#include <Core/default_resources.hpp>
#include <Core/etl/optional.hpp>
#include <Core/etl/variant.hpp>
#include <Core/etl/vector.hpp>
#include <Core/filesystem/path.hpp>
#include <Core/importer.hpp>
#include <Core/logger.hpp>
#include <Core/math/math.hpp>
#include <Core/memory.hpp>
#include <Core/package.hpp>
#include <Core/reflection/class.hpp>
#include <Engine/world.hpp>
#include <Graphics/gpu_buffers.hpp>
#include <Graphics/mesh.hpp>
#include <Graphics/shader_compiler.hpp>
#include <Graphics/texture.hpp>
#include <Graphics/visual_material.hpp>
#include <Graphics/visual_material_nodes.hpp>
#include <Image/image.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <tiny_gltf.h>

#include <Engine/Actors/directional_light_actor.hpp>
#include <Engine/Actors/point_light_actor.hpp>
#include <Engine/Actors/spot_light_actor.hpp>
#include <Engine/Actors/static_mesh_actor.hpp>

#include <Engine/ActorComponents/directional_light_component.hpp>
#include <Engine/ActorComponents/point_light_component.hpp>
#include <Engine/ActorComponents/spot_light_component.hpp>
#include <Engine/ActorComponents/static_mesh_component.hpp>


namespace Engine::Importer
{
	struct ImporterContext {
	private:
		using Mesh = Variant<Pointer<StaticMesh>, Pointer<SkeletalMesh>>;

		struct BufferInfo {
			union
			{
				byte* data = nullptr;
				uint64_t address;
			};

			size_t stride = 0;

			inline void zeroes(size_t count) { memset(data, 0, stride * count); }

			inline byte* element(size_t index) { return data + index * stride; }

			template<typename T>
			inline T* as(size_t index)
			{
				reinterpret_cast<T*>(element(index));
			}
		};

		struct Accessors {
			static constexpr uint64_t s_position_flag  = 1 << 0;
			static constexpr uint64_t s_texcoord0_flag = 1 << 1;
			static constexpr uint64_t s_normals_flag   = 1 << 2;
			static constexpr uint64_t s_tangents_flag  = 1 << 3;
			static constexpr uint64_t s_colors_flag    = 1 << 4;
			static constexpr uint64_t s_indices_flag   = 1 << 5;

			static constexpr uint64_t s_texcoord0_normal_tangent_mask = s_texcoord0_flag | s_normals_flag | s_tangents_flag;

			const tinygltf::Accessor* positions  = nullptr;
			const tinygltf::Accessor* texcoords0 = nullptr;
			const tinygltf::Accessor* normals    = nullptr;
			const tinygltf::Accessor* tangents   = nullptr;
			const tinygltf::Accessor* colors     = nullptr;
			const tinygltf::Accessor* indices    = nullptr;

			template<typename... T>
			static inline uint64_t static_mask(const T*... accessors)
			{
				uint64_t result = 0;
				size_t index    = 0;
				result |= ((result |= (accessors ? (1ull << index) : 0ull), ++index), ...);
				return result;
			}

			inline uint64_t mask() const { return static_mask(positions, texcoords0, normals, tangents, colors, indices); }
		};

	private:
		World* m_world;

		struct Packages {
			Package* root;
			Package* textures  = nullptr;
			Package* materials = nullptr;
			Package* meshes    = nullptr;

			Packages(Package* pkg) : root(pkg) {}
			Package* create_subpackage(const String& name) { return Object::new_instance<Package>(name, root); }

		} m_package;

		Matrix4f m_transform;

		Vector<Pointer<VisualMaterial>> m_materials;
		Vector<Pointer<Texture2D>> m_textures;
		Vector<Optional<Mesh>> m_meshes;

	private:
		static Matrix4f make_matrix(const tinygltf::Node& node)
		{
			Transform transform;

			if (!node.translation.empty())
			{
				const double* data = node.translation.data();
				transform.location = {data[0], data[1], data[2]};
			}

			if (!node.scale.empty())
			{
				const double* data = node.scale.data();
				transform.scale    = {data[0], data[1], data[2]};
			}

			return transform.matrix();
		}

		static const tinygltf::Accessor* find_accessor(const tinygltf::Model& model, const std::map<std::string, int>& attributes,
		                                               const std::string& attribute)
		{
			auto it = attributes.find(attribute);

			if (it == attributes.end())
				return nullptr;

			return &model.accessors[it->second];
		}

		template<typename... Accessors>
		static inline size_t elements_count(const Accessors*... accessors)
		{
			return ((accessors ? accessors->count : 0) + ...);
		}

		static inline RHIPrimitiveTopology topology_of(uint8_t mode)
		{
			switch (mode)
			{
				case TINYGLTF_MODE_POINTS: return RHIPrimitiveTopology::PointList;
				case TINYGLTF_MODE_LINE: return RHIPrimitiveTopology::LineList;
				case TINYGLTF_MODE_LINE_STRIP: return RHIPrimitiveTopology::LineStrip;
				case TINYGLTF_MODE_TRIANGLE_STRIP: return RHIPrimitiveTopology::TriangleStrip;
				default: return RHIPrimitiveTopology::TriangleList;
			}
		}

		static inline const byte* buffer_address(const tinygltf::Model& model, const tinygltf::Accessor* accessor, size_t& stride)
		{
			if (accessor->bufferView == -1)
			{
				stride = 0;
				return nullptr;
			}

			const tinygltf::BufferView& view = model.bufferViews[accessor->bufferView];
			const tinygltf::Buffer& buffer   = model.buffers[view.buffer];

			const byte* data = buffer.data.data();
			stride           = view.byteStride;
			return data + view.byteOffset + accessor->byteOffset;
		}

		static void byte2_to_float2(void* dst, const void* src)
		{
			float* destination = reinterpret_cast<float*>(dst);
			const byte* source = static_cast<const byte*>(src);

			destination[0] = static_cast<float>(source[0]) / 255.f;
			destination[1] = static_cast<float>(source[1]) / 255.f;
		}

		static void ushort2_to_float2(void* dst, const void* src)
		{
			float* destination     = reinterpret_cast<float*>(dst);
			const ushort_t* source = static_cast<const ushort_t*>(src);

			destination[0] = static_cast<float>(source[0]) / 65535.f;
			destination[1] = static_cast<float>(source[1]) / 65535.f;
		}

		static void float3_to_short3(void* dst, const void* src)
		{
			short_t* destination = reinterpret_cast<short_t*>(dst);
			const float* source  = static_cast<const float*>(src);

			for (int i = 0; i < 3; ++i)
			{
				float clamped  = Math::clamp(-1.0f, 1.0f, source[i]);
				destination[i] = static_cast<short_t>(Math::round(clamped * 32767.0f));
			}
		}

		static void float4_to_short4(void* dst, const void* src)
		{
			short_t* destination = reinterpret_cast<short_t*>(dst);
			const float* source  = static_cast<const float*>(src);

			for (int i = 0; i < 4; ++i)
			{
				float clamped  = Math::clamp(-1.f, 1.f, source[i]);
				destination[i] = static_cast<short_t>(Math::round(clamped * 32767.0f));
			}
		}

		static uint16_t find_material_index(Vector<MaterialInterface*>& materials, Material* material)
		{
			uint16_t index = 0;

			for (uint16_t count = materials.size(); index < count; ++index)
			{
				MaterialInterface* current = materials[index];

				if (current == material)
					return index;
			}

			materials.push_back(material);
			return index;
		}

		static RHIColorFormat parse_format(int component, int bits, int pixel_type)
		{
			switch (bits)
			{
				case 8:
					if (pixel_type == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
					{
						switch (component)
						{
							case 1: return RHIColorFormat::R8;
							case 2: return RHIColorFormat::R8G8;
							case 4: return RHIColorFormat::R8G8B8A8;
						}
					}
					break;

				case 16:
					if (pixel_type == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
					{
						switch (component)
						{
							switch (component)
							{
								case 1: return RHIColorFormat::R16;
								case 2: return RHIColorFormat::R16G16;
								case 4: return RHIColorFormat::R16G16B16A16;
							}
						}
					}
					break;

				case 32:
					if (pixel_type == TINYGLTF_COMPONENT_TYPE_FLOAT)
					{
						switch (component)
						{
							case 1: return RHIColorFormat::R32F;
							case 2: return RHIColorFormat::R32G32F;
							case 4: return RHIColorFormat::R32G32B32A32F;
						}
					}
					break;
			}

			return RHIColorFormat::Undefined;
		}

	public:
		ImporterContext(World* world, Package* package, const Transform& transform)
		    : m_world(world), m_package(package), m_transform(transform.matrix())
		{}

		static inline uint32_t calculate_mip_count(uint32_t width, uint32_t height)
		{
			uint32_t dimension = Math::max(width, height);
			return Math::log2<float>(dimension) + 1;
		}

		Texture2D* import_texture(const tinygltf::Model& model, int_t index)
		{
			Pointer<Texture2D>& texture = m_textures[index];

			if (texture)
				return texture;

			if (m_package.textures == nullptr)
			{
				m_package.textures = m_package.create_subpackage("Textures");
			}

			const tinygltf::Image& gltf_image = model.images[index];

			StringView name = gltf_image.name;

			if (name.empty())
			{
				name = Path(gltf_image.uri).filename();
			}

			texture = Object::new_instance<Texture2D>(name, m_package.textures);
			texture->preload();

			auto& mip  = texture->mips.emplace_back();
			mip.size.x = gltf_image.width;
			mip.size.y = gltf_image.height;
			mip.data.resize(gltf_image.image.size());


			memcpy(mip.data.data(), gltf_image.image.data(), gltf_image.image.size());

			texture->format = parse_format(gltf_image.component, gltf_image.bits, gltf_image.pixel_type);

			texture->postload();

			return texture;
		}

		VisualMaterialGraph::SampleTexture* import_sampler(VisualMaterial* material, const tinygltf::Model& model, int_t index,
		                                                   int_t uv = 0)
		{
			auto sample = material->create_node<VisualMaterialGraph::SampleTexture>();

			const tinygltf::Texture& gltf_texture = model.textures[index];

			sample->texture = import_texture(model, gltf_texture.source);
			return sample;
		}

		Material* import_material(const tinygltf::Model& model, int_t index)
		{
			Pointer<VisualMaterial>& material = m_materials[index];

			if (material)
				return material;

			if (m_package.materials == nullptr)
			{
				m_package.materials = m_package.create_subpackage("Materials");
			}

			// static auto float_node_class  = Refl::Class::static_require("Engine::VisualMaterialGraph::ConstantFloat");
			// static auto float2_node_class = Refl::Class::static_require("Engine::VisualMaterialGraph::ConstantFloat2");
			static auto float3_node_class = Refl::Class::static_require("Engine::VisualMaterialGraph::ConstantFloat3");
			// static auto float4_node_class = Refl::Class::static_require("Engine::VisualMaterialGraph::ConstantFloat4");
			static auto mul_node_class = Refl::Class::static_require("Engine::VisualMaterialGraph::Mul");


			const tinygltf::Material& gltf_material = model.materials[index];

			material = Object::new_instance<VisualMaterial>(gltf_material.name, m_package.materials);
			{
				auto root = material->root_node();

				// Base color initialization
				{
					auto& pbr_mr = gltf_material.pbrMetallicRoughness;

					Vector3f* factor = &root->base_color->default_value()->ref<Vector3f>();

					if (pbr_mr.baseColorTexture.index != -1)
					{
						auto& base_color = pbr_mr.baseColorTexture;
						auto constant    = material->create_node(float3_node_class);
						auto mul         = material->create_node(mul_node_class);

						auto sample = import_sampler(material, model, base_color.index, base_color.texCoord);

						mul->inputs()[0]->link(sample->rgba_pin());
						mul->inputs()[1]->link(constant->outputs()[0]);

						sample->texture = import_texture(model, pbr_mr.baseColorTexture.index);

						root->base_color->link(mul->outputs()[0]);
						factor = &constant->outputs()[0]->default_value()->ref<Vector3f>();
					}

					factor->r = pbr_mr.baseColorFactor[0];
					factor->g = pbr_mr.baseColorFactor[1];
					factor->b = pbr_mr.baseColorFactor[2];
				}
			}

			material->compile();
			return material;
		}

		StaticMesh* import_static_mesh(const tinygltf::Model& model, int_t index)
		{
			if (m_meshes[index].has_value())
				return etl::get<Pointer<StaticMesh>>(m_meshes[index].value());

			if (m_package.meshes == nullptr)
			{
				m_package.meshes = m_package.create_subpackage("Meshes");
			}

			StackByteAllocator::Mark mark;

			const tinygltf::Mesh& gltf_mesh = model.meshes[index];
			const size_t primitives         = gltf_mesh.primitives.size();

			StaticMesh* mesh = Object::new_instance<StaticMesh>(gltf_mesh.name, m_package.meshes);
			mesh->bounds     = {{-1, -1, -1}, {1, 1, 1}};
			mesh->materials.reserve(primitives);

			auto& lod = mesh->lods.emplace_back();
			lod.surfaces.resize(primitives);

			StackVector<Accessors> accessors(primitives);

			uint64_t accessor_mask = 0;
			size_t vertex_count    = 0;
			size_t index_count     = 0;

			for (size_t i = 0; i < primitives; ++i)
			{
				const tinygltf::Primitive& primitive = gltf_mesh.primitives[i];
				Accessors& accessor                  = accessors[i];

				accessor.positions  = find_accessor(model, primitive.attributes, "POSITION");
				accessor.texcoords0 = find_accessor(model, primitive.attributes, "TEXCOORD_0");
				accessor.normals    = find_accessor(model, primitive.attributes, "NORMAL");
				accessor.tangents   = find_accessor(model, primitive.attributes, "TANGENT");
				accessor.colors     = find_accessor(model, primitive.attributes, "COLOR_0");

				if (primitive.indices >= 0)
				{
					accessor.indices = &model.accessors[primitive.indices];
				}

				accessor_mask |= accessor.mask();

				size_t vertices = elements_count(accessor.positions, accessor.texcoords0, accessor.normals, accessor.tangents,
				                                 accessor.colors);
				size_t indices  = elements_count(accessor.indices);


				MeshSurface* surface  = &lod.surfaces[i];
				surface->topology     = topology_of(primitive.mode);
				surface->first_vertex = vertex_count;

				if (indices > 0)
				{
					surface->first_index    = index_count;
					surface->vertices_count = indices;

					vertex_count += vertices;
					index_count += indices;
				}
				else
				{
					surface->vertices_count = vertices;
					vertex_count += surface->vertices_count;
				}

				Material* material = DefaultResources::Materials::base_pass;

				if (primitive.material != -1)
					material = import_material(model, primitive.material);

				surface->material_index = find_material_index(mesh->materials, material);
			}

			BufferInfo position, uv0, normal, tangent, indices;

			if (accessor_mask & Accessors::s_position_flag)
			{
				MeshVertexAttribute attribute;

				attribute.semantic = RHIVertexSemantic::Position;
				attribute.format   = RHIVertexFormat::RGB32F;
				attribute.stream   = lod.buffers.size();
				attribute.offset   = 0;

				lod.attributes.insert(attribute);
				auto& buffer = lod.buffers.emplace_back();

				position.data   = buffer.allocate_data(RHIBufferCreateFlags::VertexBuffer, 12, vertex_count);
				position.stride = 12;

				position.zeroes(vertex_count);
			}

			if (accessor_mask & Accessors::s_texcoord0_normal_tangent_mask)
			{
				MeshVertexAttribute attribute;
				attribute.stream = lod.buffers.size();
				attribute.offset = 0;

				if (accessor_mask & Accessors::s_texcoord0_flag)
				{
					attribute.semantic = RHIVertexSemantic::TexCoord0;
					attribute.format   = RHIVertexFormat::RG32F;
					lod.attributes.insert(attribute);
					uv0.address = attribute.offset;
					attribute.offset += 8;
				}

				if (accessor_mask & Accessors::s_normals_flag)
				{
					attribute.semantic = RHIVertexSemantic::Normal;
					attribute.format   = RHIVertexFormat::RGB32F;
					lod.attributes.insert(attribute);
					normal.address = attribute.offset;
					attribute.offset += 12;
				}

				if (accessor_mask & Accessors::s_tangents_flag)
				{
					attribute.semantic = RHIVertexSemantic::Tangent;
					attribute.format   = RHIVertexFormat::RGBA32F;
					lod.attributes.insert(attribute);
					tangent.address = attribute.offset;
					attribute.offset += 16;
				}

				auto& buffer = lod.buffers.emplace_back();
				byte* data   = buffer.allocate_data(RHIBufferCreateFlags::VertexBuffer, attribute.offset, vertex_count);
				memset(data, 0, vertex_count * attribute.offset);

				uv0.data     = data + uv0.address;
				normal.data  = data + normal.address;
				tangent.data = data + tangent.address;

				uv0.stride     = attribute.offset;
				normal.stride  = attribute.offset;
				tangent.stride = attribute.offset;
			}

			if (index_count > 0)
			{
				RHIIndexFormat format = vertex_count > 0xFFFF ? RHIIndexFormat::UInt32 : RHIIndexFormat::UInt16;

				indices.data   = lod.indices.allocate_data(RHIBufferCreateFlags::IndexBuffer, format, index_count);
				indices.stride = format.stride();
				indices.zeroes(index_count);
			}

			vertex_count = 0;
			index_count  = 0;

			for (size_t i = 0; i < primitives; ++i)
			{
				Accessors& accessor = accessors[i];

				if (accessor.positions)
				{
					size_t stride;

					byte* dst       = position.element(vertex_count);
					const byte* src = buffer_address(model, accessor.positions, stride);
					memcpy_elements(dst, src, 12, accessor.positions->count, position.stride, stride);
				}

				if (accessor.texcoords0)
				{
					size_t stride;

					byte* dst       = uv0.element(vertex_count);
					const byte* src = buffer_address(model, accessor.texcoords0, stride);

					switch (accessor.texcoords0->componentType)
					{
						case TINYGLTF_COMPONENT_TYPE_BYTE:
						{
							if (stride == 0)
								stride = 2;

							memcpy_transform(dst, src, accessor.texcoords0->count, uv0.stride, stride, byte2_to_float2);
							break;
						}

						case TINYGLTF_COMPONENT_TYPE_SHORT:
						{
							if (stride == 0)
								stride = 4;

							memcpy_transform(dst, src, accessor.texcoords0->count, uv0.stride, stride, ushort2_to_float2);
							break;
						}

						default:
						{
							memcpy_elements(dst, src, 8, accessor.texcoords0->count, uv0.stride, stride);
							break;
						}
					}
				}

				if (accessor.normals)
				{
					size_t stride;

					byte* dst       = normal.element(vertex_count);
					const byte* src = buffer_address(model, accessor.normals, stride);
					memcpy_elements(dst, src, 12, accessor.normals->count, normal.stride, stride);
				}

				if (accessor.tangents)
				{
					size_t stride;

					byte* dst       = tangent.element(vertex_count);
					const byte* src = buffer_address(model, accessor.tangents, stride);
					memcpy_elements(dst, src, 16, accessor.tangents->count, tangent.stride, stride);
				}

				if (accessor.indices)
				{
					size_t stride;

					byte* dst       = indices.element(index_count);
					const byte* src = buffer_address(model, accessor.indices, stride);
					memcpy_elements(dst, src, indices.stride, accessor.indices->count, indices.stride, stride);
				}

				size_t vertices = elements_count(accessor.positions, accessor.texcoords0, accessor.normals, accessor.tangents,
				                                 accessor.colors);
				size_t indices  = elements_count(accessor.indices);

				vertex_count += vertices;
				index_count += indices;
			}

			mesh->init_render_resources();
			m_meshes[index] = mesh;
			return mesh;
		}

		SkeletalMesh* import_skeletal_mesh(const tinygltf::Model& model, int_t index, int_t skin)
		{
			Optional<Mesh>& mesh = m_meshes[index];

			if (mesh.has_value())
				return etl::get<Pointer<SkeletalMesh>>(mesh.value());

			return nullptr;
		}

		void spawn_mesh(const String& name, StaticMesh* mesh, const Matrix4f& transform)
		{
			if (m_world == nullptr)
				return;

			StaticMeshActor* actor = m_world->spawn_actor<StaticMeshActor>({}, {}, {1, 1, 1}, name);
			actor->scene_component()->local_transform(transform);
			actor->mesh_component()->mesh(mesh);
		}

		void spawn_light(const String& name, const tinygltf::Model& model, int_t index, const Matrix4f& transform)
		{
			if (m_world == nullptr)
				return;

			const tinygltf::Light& light = model.lights[index];

			PointLightActor* actor = m_world->spawn_actor<PointLightActor>({}, {}, {1, 1, 1}, name);
			actor->scene_component()->local_transform(transform);

			actor->point_light_component()->intensity(light.intensity, LightUnits::Candelas);
		}

		void import_node(const tinygltf::Model& model, const tinygltf::Node& node, const Matrix4f& transform = Matrix4f(1.f))
		{
			Matrix4f local = transform * make_matrix(node);

			if (node.mesh >= 0)
			{
				if (node.skin >= 0)
				{
					import_skeletal_mesh(model, node.mesh, node.skin);
				}
				else
				{
					spawn_mesh(node.name, import_static_mesh(model, node.mesh), local);
				}
			}

			if (node.light >= 0)
			{
				spawn_light(node.name, model, node.light, local);
			}

			for (int node : node.children)
			{
				import_node(model, model.nodes[node], local);
			}
		}

		void import_scene(const tinygltf::Model& model, const tinygltf::Scene& scene)
		{
			for (int node : scene.nodes)
			{
				import_node(model, model.nodes[node], m_transform);
			}
		}

		void import(const Path& path)
		{
			tinygltf::Model model;
			tinygltf::TinyGLTF loader;
			std::string err;
			std::string warn;

			bool ok = false;
			if (path.extension() == ".glb")
				ok = loader.LoadBinaryFromFile(&model, &err, &warn, path.c_str());
			else
				ok = loader.LoadASCIIFromFile(&model, &err, &warn, path.c_str());

			if (!ok)
			{
				error_log("Importer", "Failed to load scene!");
			}

			if (!warn.empty())
			{
				warn_log("Importer", "%s", warn.c_str());
			}

			if (!err.empty())
			{
				error_log("Importer", "%s", err.c_str());
			}

			m_meshes.resize(model.meshes.size());
			m_textures.resize(model.textures.size());
			m_materials.resize(model.materials.size());

			for (const tinygltf::Scene& scene : model.scenes)
			{
				import_scene(model, scene);
			}
		}
	};

	void import_scene(Package* package, const Path& file, World* world, const Transform& transform)
	{
		ImporterContext context(world, package, transform);
		context.import(file);
	}
}// namespace Engine::Importer
