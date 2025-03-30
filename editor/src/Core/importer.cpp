#include <Core/default_resources.hpp>
#include <Core/filesystem/path.hpp>
#include <Core/importer.hpp>
#include <Core/logger.hpp>
#include <Core/package.hpp>
#include <Graphics/gpu_buffers.hpp>
#include <Graphics/mesh.hpp>
#include <Graphics/shader_compiler.hpp>
#include <Graphics/texture_2D.hpp>
#include <Graphics/visual_material.hpp>
#include <Graphics/visual_material_graph.hpp>
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

			VtxBuffer(T* ptr, byte stride = sizeof(T)) : ptr(ptr) {}

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

		Texture2D* load_texture(StringView path)
		{
			Pointer<Texture2D>& texture_ref = textures[path];

			if (texture_ref != nullptr)
				return texture_ref.ptr();

			Path texture_path = dir / path;
			StringView name   = texture_path.stem();
			info_log("Importer", "Loading texture: %s\n", texture_path.c_str());

			Image image;

			if (image.load(texture_path).empty())
			{
				error_log("Importer", "Failed to load texture: %s\n", texture_path.c_str());
				return nullptr;
			}

			auto texture    = Object::new_instance<Texture2D>(name, package);
			texture_ref     = texture;
			texture->format = image.format();
			auto& mip       = texture->mips.emplace_back();
			mip.size        = image.size();
			mip.data        = image.buffer();
			texture->init_render_resources();

			return texture;
		}

		Material* create_material(const aiScene* scene, aiMaterial* ai_material)
		{
			return DefaultResources::Materials::base_pass;
			// VisualMaterial* material = Object::new_instance<VisualMaterial>(ai_material->GetName().C_Str(), package);
			// auto* root               = Object::instance_cast<VisualMaterialGraph::Root>(material->nodes()[0].ptr());

			// aiColor4D diffuse;
			// if (ai_material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse) == AI_SUCCESS)
			// {
			// 	root->base_color()->value = {diffuse.r, diffuse.g, diffuse.b};
			// 	root->opacity()->value    = diffuse.a;
			// }

			// aiString texture_path;


			// if (ai_material->GetTexture(aiTextureType_BASE_COLOR, 0, &texture_path) == AI_SUCCESS)
			// {
			// 	Texture2D* texture = load_texture(StringView(texture_path.C_Str(), texture_path.length));
			// 	if (texture)
			// 	{
			// 		auto node     = material->create_node<VisualMaterialGraph::Texture2D>();
			// 		node->texture = texture;
			// 		root->base_color()->create_link(node->output_pin(0));
			// 		node->name("base_color");
			// 	}
			// }

			// if (ai_material->GetTexture(aiTextureType_METALNESS, 0, &texture_path) == AI_SUCCESS)
			// {
			// 	Texture2D* texture = load_texture(StringView(texture_path.C_Str(), texture_path.length));
			// 	if (texture)
			// 	{
			// 		auto node     = material->create_node<VisualMaterialGraph::Texture2D>();
			// 		node->texture = texture;
			// 		root->metalness()->create_link(node->output_pin(1));
			// 		node->name("metalness");
			// 	}
			// }

			// if (ai_material->GetTexture(aiTextureType_DIFFUSE_ROUGHNESS, 0, &texture_path) == AI_SUCCESS)
			// {
			// 	Texture2D* texture = load_texture(StringView(texture_path.C_Str(), texture_path.length));
			// 	if (texture)
			// 	{
			// 		auto node     = material->create_node<VisualMaterialGraph::Texture2D>();
			// 		node->texture = texture;
			// 		root->roughness()->create_link(node->output_pin(2));
			// 		node->name("roughness");
			// 	}
			// }


			// if (ai_material->GetTexture(aiTextureType_NORMALS, 0, &texture_path) == AI_SUCCESS)
			// {
			// 	Texture2D* texture = load_texture(StringView(texture_path.C_Str(), texture_path.length));
			// 	if (texture)
			// 	{
			// 		auto node     = material->create_node<VisualMaterialGraph::Texture2D>();
			// 		node->texture = texture;
			// 		root->normal()->create_link(node->output_pin(0));
			// 		node->name("normal_map");
			// 	}
			// }

			// ShaderCompiler::instance()->compile(material);

			// return material;
		}

		void load_static_meshes(const aiScene* scene, const aiNode* node)
		{
			for (unsigned int i = 0, count = node->mNumChildren; i < count; ++i)
			{
				auto child = node->mChildren[i];
				load_static_meshes(scene, child);
			}

			unsigned int meshes_count = node->mNumMeshes;
			if (meshes_count == 0 || node->mMeshes == nullptr)
				return;

			unsigned int vertex_count = 0;
			unsigned int faces_count  = 0;

			for (unsigned int mesh_index = 0; mesh_index < meshes_count; ++mesh_index)
			{
				unsigned int scene_mesh_index = node->mMeshes[mesh_index];
				aiMesh* mesh                  = scene->mMeshes[scene_mesh_index];
				vertex_count += mesh->mNumVertices;
				faces_count += mesh->mNumFaces;
			}

			StaticMesh* static_mesh = Object::new_instance<StaticMesh>(node->mName.C_Str(), package);
			static_mesh->materials.resize(meshes_count);

			static_mesh->lods.resize(1);
			auto& lod = static_mesh->lods[0];
			lod.surfaces.resize(meshes_count);

			auto& bounds                = static_mesh->bounds;
			RHIIndexFormat index_format = vertex_count > 65535 ? RHIIndexFormat::UInt32 : RHIIndexFormat::UInt16;
			byte index_size             = vertex_count > 65535 ? 4 : 2;

			VtxBuffer<Vector3f> positions  = lod.positions.emplace_back().allocate_data(RHIBufferType::Static, vertex_count);
			VtxBuffer<Vector3f> normals    = lod.normals.emplace_back().allocate_data(RHIBufferType::Static, vertex_count);
			VtxBuffer<Vector3f> tangents   = lod.tangents.emplace_back().allocate_data(RHIBufferType::Static, vertex_count);
			VtxBuffer<Vector3f> bitangents = lod.bitangents.emplace_back().allocate_data(RHIBufferType::Static, vertex_count);
			VtxBuffer<Vector2f> uvs        = lod.tex_coords.emplace_back().allocate_data(RHIBufferType::Static, vertex_count);
			IdxBuffer indices(lod.indices.allocate_data(RHIBufferType::Static, index_format, faces_count * 3), index_size);

			for (unsigned int mesh_index = 0; mesh_index < meshes_count; ++mesh_index)
			{
				unsigned int scene_mesh_index = node->mMeshes[mesh_index];
				aiMesh* mesh                  = scene->mMeshes[scene_mesh_index];
				aiVector3f* texture_coords    = mesh->mTextureCoords[0];

				MeshSurface& surface      = lod.surfaces[mesh_index];
				surface.base_vertex_index = positions.size;
				surface.first_index       = indices.size;

				for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
				{
					positions.push_back(vector_cast(transform * Vector4f(vector_from_assimp_vec(mesh->mVertices[i]), 1.f)));
					normals.push_back(glm::normalize(rotation * vector_from_assimp_vec(mesh->mNormals[i])));
					tangents.push_back(glm::normalize(rotation * vector_from_assimp_vec(mesh->mTangents[i])));
					bitangents.push_back(glm::normalize(rotation * vector_from_assimp_vec(mesh->mBitangents[i])));
					uvs.push_back(vector_from_assimp_vec(texture_coords[i]));
				}

				for (unsigned int face_index = 0; face_index < mesh->mNumFaces; ++face_index)
				{
					auto& face = mesh->mFaces[face_index];
					for (unsigned int index = 0; index < face.mNumIndices; ++index)
					{
						indices.push_back(face.mIndices[index]);
					}
				}

				surface.vertices_count = indices.size - surface.first_index;

				auto& material         = static_mesh->materials[mesh_index];
				material.surface_index = mesh_index;
				material.material      = create_material(scene, scene->mMaterials[mesh->mMaterialIndex]);

				auto min_pos = vector_from_assimp_vec(mesh->mAABB.mMin);
				auto max_pos = vector_from_assimp_vec(mesh->mAABB.mMax);

				bounds.min(glm::min(min_pos, bounds.min()));
				bounds.max(glm::max(max_pos, bounds.max()));
			}

			static_mesh->init_render_resources();
		}

		void import(const Path& path)
		{
			info_log("Importer", "Loading resources from file '%s'", path.c_str());

			Assimp::Importer importer;

			unsigned int flags = aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_GenSmoothNormals |
								 aiProcess_OptimizeMeshes | aiProcess_OptimizeGraph | aiProcess_GenBoundingBoxes |
								 aiProcess_CalcTangentSpace | aiProcess_GenUVCoords;
			const aiScene* scene = importer.ReadFile(path.c_str(), flags);

			if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
			{
				error_log("Importer", "Failed to load resource: %s", importer.GetErrorString());
				return;
			}

			dir = path.base_path();
			load_static_meshes(scene, scene->mRootNode);
			importer.FreeScene();
		}
	};


	void import_resource(Package* package, const Path& file, const Transform& transform)
	{
		ImporterContext context(package, transform);
		context.import(file);
	}
}// namespace Engine::Importer
