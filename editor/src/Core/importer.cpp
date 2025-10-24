#include <Core/default_resources.hpp>
#include <Core/etl/function.hpp>
#include <Core/filesystem/path.hpp>
#include <Core/importer.hpp>
#include <Core/logger.hpp>
#include <Core/math/math.hpp>
#include <Core/package.hpp>
#include <Core/thread_manager.hpp>
#include <Core/threading.hpp>
#include <Engine/ActorComponents/static_mesh_component.hpp>
#include <Engine/Actors/static_mesh_actor.hpp>
#include <Engine/world.hpp>
#include <Graphics/gpu_buffers.hpp>
#include <Graphics/mesh.hpp>
#include <Graphics/shader_compiler.hpp>
#include <Graphics/texture.hpp>
#include <Graphics/visual_material.hpp>
#include <Graphics/visual_material_nodes.hpp>
#include <Image/image.hpp>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

namespace Engine::Importer
{
	struct ImporterContext {
		template<typename T>
		struct VtxBuffer {
			T* ptr;
			size_t size = 0;

			VtxBuffer(byte* ptr, byte stride = sizeof(T)) : ptr(reinterpret_cast<T*>(ptr)) {}

			inline void push_back(T&& value)
			{
				new (ptr++) T(std::move(value));
				++size;
			}

			inline void push_back(const T& value)
			{
				new (ptr++) T(value);
				++size;
			}
		};

		struct IdxBuffer {
		private:
			byte* (*m_push)(byte*, uint32_t);

			template<typename T>
			static byte* push(byte* ptr, uint32_t value)
			{
				new (ptr) T(static_cast<T>(value));
				return ptr + sizeof(T);
			}

		public:
			byte* ptr;
			size_t size = 0;

			IdxBuffer(void* ptr, byte stride) : ptr(static_cast<byte*>(ptr))
			{
				m_push = stride == 2 ? push<uint16_t> : push<uint32_t>;
			}

			inline void push_back(uint32_t value)
			{
				ptr = m_push(ptr, value);
				++size;
			}
		};


		Package* package;
		Matrix4f transform;
		Matrix3f rotation;
		Map<StringView, Pointer<Texture2D>> textures;
		Map<aiMaterial*, Pointer<VisualMaterial>> materials;
		Path dir;

		ImporterContext(Package* package, const Transform& transform)
		    : package(package), transform(transform.matrix()), rotation(transform.rotation_matrix())
		{}

		static inline Vector3f vector_from_assimp_vec(const aiVector3f& vector)
		{
			Vector3f result;
			result.x = vector.x;
			result.y = vector.y;
			result.z = vector.z;
			return result;
		}

		static inline Vector3f vector_cast(const Vector4f& vector)
		{
			return {vector.x / vector.w, vector.y / vector.w, vector.z / vector.w};
		}

		static inline Vector4f pack_tangent(const Vector3f& N, const Vector3f& T, const Vector3f& B)
		{
			float handedness = Math::sign(Math::dot(Math::cross(N, T), B));
			return Vector4f(T.x, T.y, T.z, handedness);
		}

		Matrix4f matrix_cast(const aiMatrix4x4& m)
		{
			Matrix4f result;

			// clang-format off
			result[0][0] = m.a1; result[1][0] = m.a2; result[2][0] = m.a3; result[3][0] = m.a4;
			result[0][1] = m.b1; result[1][1] = m.b2; result[2][1] = m.b3; result[3][1] = m.b4;
			result[0][2] = m.c1; result[1][2] = m.c2; result[2][2] = m.c3; result[3][2] = m.c4;
			result[0][3] = m.d1; result[1][3] = m.d2; result[2][3] = m.d3; result[3][3] = m.d4;
			// clang-format on

			return result;
		}

		static inline uint32_t calculate_mip_count(uint32_t width, uint32_t height)
		{
			uint32_t dimension = Math::max(width, height);
			return Math::log2<float>(dimension) + 1;
		}

		Texture2D* load_texture(const aiScene* scene, int_t index)
		{
			aiTexture* ai_texture = scene->mTextures[index];
			String name           = Strings::format("Texture {}", static_cast<void*>(ai_texture));

			if (ai_texture->mHeight > 0)
			{
				auto texture = Object::new_instance<Texture2D>(name, package);

				texture->format = RHIColorFormat::R8G8B8A8;
				auto& mip       = texture->mips.emplace_back();

				mip.size = Vector2u(ai_texture->mWidth, ai_texture->mHeight);
				mip.data.resize(ai_texture->mWidth * ai_texture->mHeight * 4);
				std::memcpy(mip.data.data(), ai_texture->pcData, mip.data.size());

				texture->init_render_resources();
				return texture;
			}
			else
			{
				Image image = Image(ai_texture->pcData, ai_texture->mWidth);

				if (image.is_empty())
					return DefaultResources::Textures::default_texture;

				auto texture = Object::new_instance<Texture2D>(name, package);

				texture->format = image.format();
				auto& mip       = texture->mips.emplace_back();
				mip.size        = image.size();
				mip.data        = image.buffer();
				texture->init_render_resources();
				return texture;
			}
		}

		Texture2D* load_texture(const aiScene* scene, StringView path)
		{
			Pointer<Texture2D>& texture_ref = textures[path];

			if (texture_ref != nullptr)
				return texture_ref.ptr();

			if (path.front() == '*')
			{
				int_t index = Strings::integer_of(path.data() + 1);
				texture_ref = load_texture(scene, index);
				return texture_ref;
			}

			Path texture_path = dir / path;
			StringView name   = texture_path.stem();
			info_log("Importer", "Loading texture: %s\n", texture_path.c_str());

			Image image = texture_path;

			if (image.is_empty())
			{
				error_log("Importer", "Failed to load texture: %s\n", texture_path.c_str());
				return DefaultResources::Textures::default_texture;
			}

			auto texture = Object::new_instance<Texture2D>(name, package);
			texture_ref  = texture;

			texture->format = RHIColorFormat::R8G8B8A8;

			const uint_t mips_count = calculate_mip_count(image.width(), image.height());

			Vector<Texture2DMip> mips;
			mips.resize(mips_count);

			mips[0].size = image.size();
			mips[0].data = image.buffer();

			for (uint_t i = 1; i < mips_count; ++i)
			{
				auto& mip = mips[i];

				uint_t width  = Math::max<uint32_t>(image.width() / 2, 1);
				uint_t height = Math::max<uint32_t>(image.height() / 2, 1);
				image.resize({width, height});

				mip.size = {width, height};
				mip.data = image.buffer();
			}

			texture->format = image.format();
			texture->mips   = std::move(mips);
			texture->init_render_resources();

			return texture;
		}

		Material* create_material(const aiScene* scene, aiMaterial* ai_material)
		{
			Pointer<VisualMaterial>& material = materials[ai_material];

			if (material)
				return material;

			if (ai_material->GetName().length != 0)
			{
				material = Object::new_instance<VisualMaterial>(ai_material->GetName().C_Str(), package);
			}
			else
			{
				String name = Strings::format("Material {}", static_cast<const void*>(ai_material));
				material    = Object::new_instance<VisualMaterial>(name.c_str(), package);
			}
			auto* root = material->root_node();

			aiColor4D diffuse;
			if (ai_material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse) == AI_SUCCESS)
			{
				root->base_color->default_value()->ref<Vector3f>() = {diffuse.r, diffuse.g, diffuse.b};
				root->opacity->default_value()->ref<float>()       = diffuse.a;
			}

			aiString texture_path;
			aiString roughness_path;

			if (ai_material->GetTexture(aiTextureType_BASE_COLOR, 0, &texture_path) == AI_SUCCESS)
			{
				Texture2D* texture = load_texture(scene, StringView(texture_path.C_Str(), texture_path.length));
				if (texture)
				{
					auto node     = material->create_node<VisualMaterialGraph::SampleTexture>();
					node->texture = texture;
					root->base_color->link(node->rgba_pin());
				}
			}

			if (ai_material->GetTexture(aiTextureType_METALNESS, 0, &texture_path) == AI_SUCCESS)
			{
				Texture2D* texture = load_texture(scene, StringView(texture_path.C_Str(), texture_path.length));
				if (texture)
				{
					auto node     = material->create_node<VisualMaterialGraph::SampleTexture>();
					node->texture = texture;
					root->metalness->link(node->r_pin());
				}
			}

			if (ai_material->GetTexture(aiTextureType_DIFFUSE_ROUGHNESS, 0, &roughness_path) == AI_SUCCESS)
			{
				Texture2D* texture = load_texture(scene, StringView(roughness_path.C_Str(), roughness_path.length));
				if (texture)
				{
					auto node     = material->create_node<VisualMaterialGraph::SampleTexture>();
					node->texture = texture;

					if (roughness_path == texture_path)
						root->roughness->link(node->g_pin());
					else
						root->roughness->link(node->r_pin());
				}
			}

			if (ai_material->GetTexture(aiTextureType_NORMALS, 0, &texture_path) == AI_SUCCESS)
			{
				Texture2D* texture = load_texture(scene, StringView(texture_path.C_Str(), texture_path.length));
				if (texture)
				{
					auto node     = material->create_node<VisualMaterialGraph::SampleTexture>();
					node->texture = texture;
					root->normal->link(node->rgba_pin());
				}
			}

			material->compile();
			return material;
		}

		StaticMesh* load_static_mesh(const aiScene* scene, const aiNode* node)
		{
			String name = node->mName.C_Str() ? String(node->mName.C_Str())
			                                  : Strings::format("Static Mesh {}", static_cast<const void*>(node));

			StaticMesh* static_mesh = package->find_child_object_checked<StaticMesh>(name);

			if (static_mesh)
				return static_mesh;

			unsigned int meshes_count = node->mNumMeshes;
			if (meshes_count == 0 || node->mMeshes == nullptr)
				return nullptr;

			unsigned int vertex_count = 0;
			unsigned int faces_count  = 0;

			for (unsigned int mesh_index = 0; mesh_index < meshes_count; ++mesh_index)
			{
				unsigned int scene_mesh_index = node->mMeshes[mesh_index];
				aiMesh* mesh                  = scene->mMeshes[scene_mesh_index];
				vertex_count += mesh->mNumVertices;
				faces_count += mesh->mNumFaces;
			}

			static_mesh = Object::new_instance<StaticMesh>(name.c_str(), package);
			static_mesh->materials.resize(meshes_count);

			static_mesh->lods.resize(1);
			auto& lod = static_mesh->lods[0];
			lod.surfaces.resize(meshes_count);

			auto& bounds                = static_mesh->bounds;
			RHIIndexFormat index_format = vertex_count > 65535 ? RHIIndexFormat::UInt32 : RHIIndexFormat::UInt16;
			byte index_size             = vertex_count > 65535 ? 4 : 2;

			lod.buffers.reserve(4);

			// clang-format off
			VtxBuffer<Vector3f> positions  = lod.buffers.emplace_back().allocate_data(RHIBufferCreateFlags::Static, sizeof(Vector3f), vertex_count);
			VtxBuffer<Vector3f> normals    = lod.buffers.emplace_back().allocate_data(RHIBufferCreateFlags::Static, sizeof(Vector3f), vertex_count);
			VtxBuffer<Vector4f> tangents   = lod.buffers.emplace_back().allocate_data(RHIBufferCreateFlags::Static, sizeof(Vector4f), vertex_count);
			VtxBuffer<Vector2f> uvs        = lod.buffers.emplace_back().allocate_data(RHIBufferCreateFlags::Static, sizeof(Vector2f), vertex_count);
			IdxBuffer indices(lod.indices.allocate_data(RHIBufferCreateFlags::Static, index_format, faces_count * 3), index_size);
			// clang-format on

			for (unsigned int mesh_index = 0; mesh_index < meshes_count; ++mesh_index)
			{
				unsigned int scene_mesh_index = node->mMeshes[mesh_index];
				aiMesh* mesh                  = scene->mMeshes[scene_mesh_index];
				aiVector3f* texture_coords    = mesh->mTextureCoords[0];

				MeshSurface& surface = lod.surfaces[mesh_index];

				surface.attributes = {{RHIVertexSemantic::Position, RHIVertexFormat::Float3, 0, 0},
				                      {RHIVertexSemantic::Normal, RHIVertexFormat::Float3, 1, 0},
				                      {RHIVertexSemantic::Tangent, RHIVertexFormat::Float4, 2, 0},
				                      {RHIVertexSemantic::TexCoord0, RHIVertexFormat::Float2, 3, 0}};

				surface.index_format = index_format;

				if (mesh->mPrimitiveTypes & aiPrimitiveType_TRIANGLE)
					surface.topology = RHIPrimitiveTopology::TriangleList;
				else if (mesh->mPrimitiveTypes & aiPrimitiveType_LINE)
					surface.topology = RHIPrimitiveTopology::LineList;
				else if (mesh->mPrimitiveTypes & aiPrimitiveType_POINT)
					surface.topology = RHIPrimitiveTopology::PointList;
				else
					surface.topology = RHIPrimitiveTopology::TriangleList;

				surface.base_vertex_index = positions.size;
				surface.first_index       = indices.size;

				for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
				{
					Vector3f position  = vector_cast(transform * Vector4f(vector_from_assimp_vec(mesh->mVertices[i]), 1.f));
					Vector3f normal    = Math::normalize(rotation * vector_from_assimp_vec(mesh->mNormals[i]));
					Vector3f tangent   = Math::normalize(rotation * vector_from_assimp_vec(mesh->mTangents[i]));
					Vector3f bitangent = Math::normalize(rotation * vector_from_assimp_vec(mesh->mBitangents[i]));
					Vector2f uv        = vector_from_assimp_vec(texture_coords[i]);

					positions.push_back(position);
					normals.push_back(normal);
					tangents.push_back(pack_tangent(normal, tangent, bitangent));
					uvs.push_back(uv);
				}

				for (unsigned int face_index = 0; face_index < mesh->mNumFaces; ++face_index)
				{
					auto& face = mesh->mFaces[face_index];
					for (unsigned int index = 0; index < face.mNumIndices; ++index)
					{
						indices.push_back(face.mIndices[index]);
					}
				}

				surface.vertices_count             = indices.size - surface.first_index;
				surface.material_index             = mesh_index;
				static_mesh->materials[mesh_index] = create_material(scene, scene->mMaterials[mesh->mMaterialIndex]);

				auto min_pos = vector_from_assimp_vec(mesh->mAABB.mMin);
				auto max_pos = vector_from_assimp_vec(mesh->mAABB.mMax);

				bounds.min = Math::min(min_pos, bounds.min);
				bounds.max = Math::max(max_pos, bounds.max);
			}

			static_mesh->init_render_resources();
			return static_mesh;
		}

		void load_meshes(const aiScene* scene, const aiNode* node, World* world, const Matrix4f& transform = Matrix4f(1.f))
		{
			Matrix4f node_transform = transform * matrix_cast(node->mTransformation);

			StaticMesh* mesh = load_static_mesh(scene, node);

			if (world && mesh)
			{
				StaticMeshActor* actor = world->spawn_actor<StaticMeshActor>();
				auto component         = actor->mesh_component();

				component->mesh(mesh);
				component->local_transform(node_transform);
			}

			for (unsigned int i = 0, count = node->mNumChildren; i < count; ++i)
			{
				auto child = node->mChildren[i];
				load_meshes(scene, child, world, node_transform);
			}
		}

		void import(const Path& path, World* world = nullptr)
		{
			info_log("Importer", "Loading resources from file '%s'", path.c_str());

			Assimp::Importer importer;

			unsigned int flags = aiProcess_JoinIdenticalVertices | aiProcess_LimitBoneWeights | aiProcess_Triangulate |
			                     aiProcess_SortByPType | aiProcess_GenUVCoords | aiProcess_FindDegenerates |
			                     aiProcess_FindInvalidData | aiProcess_GlobalScale | aiProcess_GenBoundingBoxes |
			                     aiProcess_FlipUVs;

			flags |= aiProcess_FixInfacingNormals | aiProcess_GenSmoothNormals;
			flags |= aiProcess_CalcTangentSpace;
			flags |= aiProcess_OptimizeMeshes | aiProcess_ImproveCacheLocality;

			const aiScene* scene = importer.ReadFile(path.c_str(), flags);

			if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
			{
				error_log("Importer", "Failed to load resource: %s", importer.GetErrorString());
				return;
			}

			dir = path.base_path();
			load_meshes(scene, scene->mRootNode, world);
			importer.FreeScene();
		}
	};

	void import_scene(Package* package, const Path& file, World* world, const Transform& transform)
	{
		ImporterContext context(package, transform);
		context.import(file, world);
	}
}// namespace Engine::Importer
