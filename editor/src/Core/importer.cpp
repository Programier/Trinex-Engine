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
#include <Core/string_functions.hpp>
#include <Engine/world.hpp>
#include <Graphics/gpu_buffers.hpp>
#include <Graphics/material.hpp>
#include <Graphics/mesh.hpp>
#include <Graphics/shader_compiler.hpp>
#include <Graphics/texture.hpp>
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


namespace Trinex::Importer
{
	struct ImporterContext {
	private:
		using Mesh = Variant<Pointer<StaticMesh>, Pointer<SkeletalMesh>>;

		struct BufferInfo {
			union
			{
				u8* data = nullptr;
				u64 address;
			};

			usize stride = 0;

			inline void zeroes(usize count) { memset(data, 0, stride * count); }

			inline u8* element(usize index) { return data + index * stride; }

			template<typename T>
			inline T* as(usize index)
			{
				reinterpret_cast<T*>(element(index));
			}
		};

		struct Accessors {
			static constexpr u64 s_position_flag  = 1 << 0;
			static constexpr u64 s_texcoord0_flag = 1 << 1;
			static constexpr u64 s_normals_flag   = 1 << 2;
			static constexpr u64 s_tangents_flag  = 1 << 3;
			static constexpr u64 s_colors_flag    = 1 << 4;
			static constexpr u64 s_indices_flag   = 1 << 5;

			static constexpr u64 s_texcoord0_normal_tangent_mask = s_texcoord0_flag | s_normals_flag | s_tangents_flag;

			const tinygltf::Accessor* positions  = nullptr;
			const tinygltf::Accessor* texcoords0 = nullptr;
			const tinygltf::Accessor* normals    = nullptr;
			const tinygltf::Accessor* tangents   = nullptr;
			const tinygltf::Accessor* colors     = nullptr;
			const tinygltf::Accessor* indices    = nullptr;

			template<typename... T>
			static inline u64 static_mask(const T*... accessors)
			{
				u64 result  = 0;
				usize index = 0;
				((result |= (accessors ? (1ull << index) : 0ull), ++index), ...);
				return result;
			}

			inline u64 mask() const { return static_mask(positions, texcoords0, normals, tangents, colors, indices); }
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

		struct MeshInfo {
			Mesh mesh;
			Vector3f offset;
		};

		Matrix4f m_transform;

		Vector<Pointer<MaterialInstance>> m_materials;
		Vector<Pointer<Texture2D>> m_textures;
		Vector<Optional<MeshInfo>> m_meshes;

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


		static inline usize accessor_element_size(const tinygltf::Accessor* accessor)
		{
			const usize component_size  = tinygltf::GetComponentSizeInBytes(accessor->componentType);
			const usize component_count = tinygltf::GetNumComponentsInType(accessor->type);
			return component_size * component_count;
		}

		template<typename... Accessors>
		static inline usize elements_count(const Accessors*... accessors)
		{
			usize max_count = 0;
			((max_count = Math::max(max_count, accessors ? accessors->count : 0)), ...);
			return max_count;
		}

		static inline RHITopology topology_of(u8 mode)
		{
			switch (mode)
			{
				case TINYGLTF_MODE_LINE: return RHITopology::LineList;
				case TINYGLTF_MODE_LINE_STRIP: return RHITopology::LineStrip;
				case TINYGLTF_MODE_TRIANGLE_STRIP: return RHITopology::TriangleStrip;
				case TINYGLTF_MODE_TRIANGLES: return RHITopology::TriangleList;
				default: trinex_assert(false);
			}
		}

		static inline const u8* buffer_address(const tinygltf::Model& model, const tinygltf::Accessor* accessor, usize& stride)
		{
			if (accessor->bufferView == -1)
			{
				stride = 0;
				return nullptr;
			}

			const tinygltf::BufferView& view = model.bufferViews[accessor->bufferView];
			const tinygltf::Buffer& buffer   = model.buffers[view.buffer];

			const u8* data = buffer.data.data();
			stride         = view.byteStride;
			return data + view.byteOffset + accessor->byteOffset;
		}

		static void byte2_to_float2(void* dst, const void* src)
		{
			float* destination = reinterpret_cast<float*>(dst);
			const u8* source   = static_cast<const u8*>(src);

			destination[0] = static_cast<float>(source[0]) / 255.f;
			destination[1] = static_cast<float>(source[1]) / 255.f;
		}

		static void ushort2_to_float2(void* dst, const void* src)
		{
			float* destination = reinterpret_cast<float*>(dst);
			const u16* source  = static_cast<const u16*>(src);

			destination[0] = static_cast<float>(source[0]) / 65535.f;
			destination[1] = static_cast<float>(source[1]) / 65535.f;
		}

		static void float3_to_short3(void* dst, const void* src)
		{
			i16* destination    = reinterpret_cast<i16*>(dst);
			const float* source = static_cast<const float*>(src);

			for (int i = 0; i < 3; ++i)
			{
				float clamped  = Math::clamp(-1.0f, 1.0f, source[i]);
				destination[i] = static_cast<i16>(Math::round(clamped * 32767.0f));
			}
		}

		static void float4_to_short4(void* dst, const void* src)
		{
			i16* destination    = reinterpret_cast<i16*>(dst);
			const float* source = static_cast<const float*>(src);

			for (int i = 0; i < 4; ++i)
			{
				float clamped  = Math::clamp(-1.f, 1.f, source[i]);
				destination[i] = static_cast<i16>(Math::round(clamped * 32767.0f));
			}
		}

		static u16 find_material_index(Vector<MaterialInterface*>& materials, MaterialInterface* material)
		{
			u16 index = 0;

			for (u16 count = materials.size(); index < count; ++index)
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

		static Box3f bounding_box(Vector3f* positions, usize count)
		{
			if (positions == nullptr || count == 0)
				return Box3f();

			Vector3f min = positions[0];
			Vector3f max = positions[0];

			for (usize i = 1; i < count; ++i)
			{
				const Vector3f& p = positions[i];

				min = Math::min(min, p);
				max = Math::max(max, p);
			}

			return Box3f(min, max);
		}

		static void offset_vertices(Vector3f* vertices, usize count, Vector3f offset)
		{
			if (Math::length(offset) < 0.000001)
				return;

			for (usize i = 0; i < count; ++i)
			{
				vertices[i] += offset;
			}
		}

	public:
		ImporterContext(World* world, Package* package, const Transform& transform)
		    : m_world(world), m_package(package), m_transform(transform.matrix())
		{}

		static inline u32 calculate_mip_count(u32 width, u32 height)
		{
			u32 dimension = Math::max(width, height);
			return Math::log2<float>(dimension) + 1;
		}

		Texture2D* import_texture(const tinygltf::Model& model, i32 index, Texture2D* base)
		{
			if (index < 0)
				return base;

			index = model.textures[index].source;

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
				name = gltf_image.name;
			}

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

		Sampler import_sampler(const tinygltf::Model& model, i32 index, i32 uv = 0)
		{
			if (index < 0)
				return Sampler(RHISamplerFilter::Point);

			index = model.textures[index].sampler;

			if (index < 0)
				return Sampler(RHISamplerFilter::Point);

			const tinygltf::Sampler& src = model.samplers[index];
			return Sampler(RHISamplerFilter::Bilinear);
		}

		MaterialInterface* import_material(const tinygltf::Model& model, i32 index)
		{
			Pointer<MaterialInstance>& material = m_materials[index];

			if (material)
				return material;

			if (m_package.materials == nullptr)
			{
				m_package.materials = m_package.create_subpackage("Materials");
			}

			const tinygltf::Material& gltf_material = model.materials[index];

			material = Object::new_instance<MaterialInstance>(gltf_material.name, m_package.materials);
			{
				namespace Parameters = MaterialParameters;

				auto& pbr = gltf_material.pbrMetallicRoughness;

				auto white_texture  = DefaultResources::Textures::white;
				auto normal_texture = DefaultResources::Textures::normal;

				// Base color initialization
				{
					auto& factor  = material->create_parameter<Parameters::Float3>("base_color.factor")->value;
					auto* texture = material->create_parameter<Parameters::Sampler2D>("base_color.texture");

					factor.r         = pbr.baseColorFactor[0];
					factor.g         = pbr.baseColorFactor[1];
					factor.b         = pbr.baseColorFactor[2];
					texture->texture = import_texture(model, pbr.baseColorTexture.index, white_texture);
					texture->sampler = import_sampler(model, pbr.baseColorTexture.index);
				}

				// Metalic-Roughness initialization
				{
					auto& factor  = material->create_parameter<Parameters::Float3>("metalic_roughness.factor")->value;
					auto* texture = material->create_parameter<Parameters::Sampler2D>("metalic_roughness.texture");

					factor.x         = pbr.metallicFactor;
					factor.y         = pbr.roughnessFactor;
					texture->texture = import_texture(model, pbr.metallicRoughnessTexture.index, white_texture);
					texture->sampler = import_sampler(model, pbr.metallicRoughnessTexture.index);
				}

				// Normal initialization
				{
					auto& normal  = gltf_material.normalTexture;
					auto* texture = material->create_parameter<Parameters::Sampler2D>("normal");

					texture->texture = import_texture(model, normal.index, normal_texture);
					texture->sampler = import_sampler(model, normal.index);
				}
			}

			return material;
		}

		StaticMesh* import_static_mesh(const tinygltf::Model& model, i32 index, Vector3f& offset)
		{
			if (m_meshes[index].has_value())
			{
				MeshInfo& info = m_meshes[index].value();
				offset         = info.offset;
				return etl::get<Pointer<StaticMesh>>(info.mesh);
			}

			if (m_package.meshes == nullptr)
			{
				m_package.meshes = m_package.create_subpackage("Meshes");
			}

			StackByteAllocator::Mark mark;

			const tinygltf::Mesh& gltf_mesh = model.meshes[index];
			const usize primitives          = gltf_mesh.primitives.size();

			StaticMesh* mesh =
			        Object::new_instance<StaticMesh>(Strings::format("{}_{}", gltf_mesh.name, index), m_package.meshes);
			mesh->materials.reserve(primitives);

			auto& lod = mesh->lods.emplace_back();
			lod.surfaces.resize(primitives);

			StackVector<Accessors> accessors(primitives);

			u64 accessor_mask  = 0;
			usize vertex_count = 0;
			usize index_count  = 0;

			for (usize i = 0; i < primitives; ++i)
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

				usize vertices = elements_count(accessor.positions, accessor.texcoords0, accessor.normals, accessor.tangents,
				                                accessor.colors);
				usize indices  = elements_count(accessor.indices);


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

				MaterialInterface* material = DefaultResources::Materials::base_pass;

				if (primitive.material != -1)
					material = import_material(model, primitive.material);

				surface->material_index = find_material_index(mesh->materials, material);
			}

			BufferInfo position, uv0, normal, tangent, indices;

			if (accessor_mask & Accessors::s_position_flag)
			{
				MeshVertexAttribute attribute;

				attribute.semantic = RHISemantic::Position;
				attribute.format   = RHIVertexFormat::RGB32F;
				attribute.stream   = lod.buffers.size();
				attribute.offset   = 0;

				lod.attributes.insert(attribute);
				auto& buffer = lod.buffers.emplace_back();

				position.data   = buffer.allocate_data(RHIBufferFlags::VertexBuffer, 12, vertex_count);
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
					attribute.semantic = RHISemantic::TexCoord0;
					attribute.format   = RHIVertexFormat::RG32F;
					lod.attributes.insert(attribute);
					uv0.address = attribute.offset;
					attribute.offset += 8;
				}

				if (accessor_mask & Accessors::s_normals_flag)
				{
					attribute.semantic = RHISemantic::Normal;
					attribute.format   = RHIVertexFormat::RGB32F;
					lod.attributes.insert(attribute);
					normal.address = attribute.offset;
					attribute.offset += 12;
				}

				if (accessor_mask & Accessors::s_tangents_flag)
				{
					attribute.semantic = RHISemantic::Tangent;
					attribute.format   = RHIVertexFormat::RGBA32F;
					lod.attributes.insert(attribute);
					tangent.address = attribute.offset;
					attribute.offset += 16;
				}

				auto& buffer = lod.buffers.emplace_back();
				u8* data     = buffer.allocate_data(RHIBufferFlags::VertexBuffer, attribute.offset, vertex_count);
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

				indices.data   = lod.indices.allocate_data(RHIBufferFlags::IndexBuffer, format, index_count);
				indices.stride = format.stride();
				indices.zeroes(index_count);
			}

			vertex_count = 0;
			index_count  = 0;

			for (usize i = 0; i < primitives; ++i)
			{
				Accessors& accessor = accessors[i];

				if (accessor.positions)
				{
					usize stride;

					u8* dst       = position.element(vertex_count);
					const u8* src = buffer_address(model, accessor.positions, stride);
					memcpy_elements(dst, src, 12, accessor.positions->count, position.stride, stride);
				}

				if (accessor.texcoords0)
				{
					usize stride;

					u8* dst       = uv0.element(vertex_count);
					const u8* src = buffer_address(model, accessor.texcoords0, stride);

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
					usize stride;

					u8* dst       = normal.element(vertex_count);
					const u8* src = buffer_address(model, accessor.normals, stride);
					memcpy_elements(dst, src, 12, accessor.normals->count, normal.stride, stride);
				}

				if (accessor.tangents)
				{
					usize stride;

					u8* dst       = tangent.element(vertex_count);
					const u8* src = buffer_address(model, accessor.tangents, stride);
					memcpy_elements(dst, src, 16, accessor.tangents->count, tangent.stride, stride);
				}

				if (accessor.indices)
				{
					usize stride;

					u8* dst            = indices.element(index_count);
					const u8* src      = buffer_address(model, accessor.indices, stride);
					usize element_size = accessor_element_size(accessor.indices);

					memcpy_elements(dst, src, element_size, accessor.indices->count, indices.stride, stride);
				}

				usize vertices = elements_count(accessor.positions, accessor.texcoords0, accessor.normals, accessor.tangents,
				                                accessor.colors);
				usize indices  = elements_count(accessor.indices);

				vertex_count += vertices;
				index_count += indices;
			}

			mesh->bounds = bounding_box(reinterpret_cast<Vector3f*>(position.data), vertex_count);
			offset       = mesh->bounds.center();
			offset_vertices(reinterpret_cast<Vector3f*>(position.data), vertex_count, -offset);
			mesh->bounds.center({0.f, 0.f, 0.f});

			mesh->init_render_resources();
			m_meshes[index] = MeshInfo{.mesh = mesh, .offset = offset};
			return mesh;
		}

		template<typename T>
		T* create_actor(const String& name, const Transform& transform)
		{
			T* actor = Object::new_instance<T>(name);
			actor->scene_component()->local_transform(transform);
			return actor;
		}

		SkeletalMesh* import_skeletal_mesh(const tinygltf::Model& model, i32 index, i32 skin)
		{
			// Optional<Mesh>& mesh = m_meshes[index];

			// if (mesh.has_value())
			// 	return etl::get<Pointer<SkeletalMesh>>(mesh.value());

			return nullptr;
		}

		void spawn_mesh(const String& name, StaticMesh* mesh, const Matrix4f& transform)
		{
			if (m_world == nullptr)
				return;

			StaticMeshActor* actor = create_actor<StaticMeshActor>(name, transform);
			actor->mesh_component()->mesh(mesh);
			actor->owner(m_world);
		}

		void spawn_light(const String& name, const tinygltf::Model& model, i32 index, const Matrix4f& transform)
		{
			if (m_world == nullptr)
				return;

			const tinygltf::Light& light = model.lights[index];

			PointLightActor* actor = create_actor<PointLightActor>(name, transform);
			actor->point_light_component()->intensity(light.intensity, LightUnits::Candelas);
			actor->owner(m_world);
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
					Vector3f offset;
					StaticMesh* mesh = import_static_mesh(model, node.mesh, offset);
					spawn_mesh(node.name, mesh, Math::translate(local, offset));
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
			m_textures.resize(model.images.size());
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
}// namespace Trinex::Importer
