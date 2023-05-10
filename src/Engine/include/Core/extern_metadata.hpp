#pragma once
#include<Core/etl/metadata.hpp>

namespace Engine
{
	class ApiObject;
	class BasicFrameBuffer;
	class Camera;
	class CommandLet;
	class FrameBuffer;
	class IndexBuffer;
	class Object;
	class OctreeBase;
	class OctreeBaseNode;
	class Package;
	class Shader;
	class Skybox;
	class Texture;
	class Texture2D;
	class TextureCubeMap;
	class VertexBuffer;
}

namespace Engine
{
#ifndef MAKEHEADERS_EXTERN_METADATA
#define MAKEHEADERS_EXTERN_METADATA
	extern template const Engine::Class* const ClassMetaData<class MakeHeaders*>::class_instance;
#endif

#ifndef ENGINE_PACKAGE_EXTERN_METADATA
#define ENGINE_PACKAGE_EXTERN_METADATA
	extern template const Engine::Class* const ClassMetaData<class Engine::Package*>::class_instance;
#endif

#ifndef ENGINE_BASICFRAMEBUFFER_EXTERN_METADATA
#define ENGINE_BASICFRAMEBUFFER_EXTERN_METADATA
	extern template const Engine::Class* const ClassMetaData<class Engine::BasicFrameBuffer*>::class_instance;
#endif

#ifndef ENGINE_INDEXBUFFER_EXTERN_METADATA
#define ENGINE_INDEXBUFFER_EXTERN_METADATA
	extern template const Engine::Class* const ClassMetaData<class Engine::IndexBuffer*>::class_instance;
#endif

#ifndef ENGINE_SHADER_EXTERN_METADATA
#define ENGINE_SHADER_EXTERN_METADATA
	extern template const Engine::Class* const ClassMetaData<class Engine::Shader*>::class_instance;
#endif

#ifndef ENGINE_COMMANDLET_EXTERN_METADATA
#define ENGINE_COMMANDLET_EXTERN_METADATA
	extern template const Engine::Class* const ClassMetaData<class Engine::CommandLet*>::class_instance;
#endif

#ifndef ENGINE_TEXTURE2D_EXTERN_METADATA
#define ENGINE_TEXTURE2D_EXTERN_METADATA
	extern template const Engine::Class* const ClassMetaData<class Engine::Texture2D*>::class_instance;
#endif

#ifndef ENGINE_TEXTURECUBEMAP_EXTERN_METADATA
#define ENGINE_TEXTURECUBEMAP_EXTERN_METADATA
	extern template const Engine::Class* const ClassMetaData<class Engine::TextureCubeMap*>::class_instance;
#endif

#ifndef ENGINE_FRAMEBUFFER_EXTERN_METADATA
#define ENGINE_FRAMEBUFFER_EXTERN_METADATA
	extern template const Engine::Class* const ClassMetaData<class Engine::FrameBuffer*>::class_instance;
#endif

#ifndef ENGINE_TEXTURE_EXTERN_METADATA
#define ENGINE_TEXTURE_EXTERN_METADATA
	extern template const Engine::Class* const ClassMetaData<class Engine::Texture*>::class_instance;
#endif

#ifndef ENGINE_OCTREEBASE_EXTERN_METADATA
#define ENGINE_OCTREEBASE_EXTERN_METADATA
	extern template const Engine::Class* const ClassMetaData<class Engine::OctreeBase*>::class_instance;
#endif

#ifndef ENGINE_OBJECT_EXTERN_METADATA
#define ENGINE_OBJECT_EXTERN_METADATA
	extern template const Engine::Class* const ClassMetaData<class Engine::Object*>::class_instance;
#endif

#ifndef ENGINE_OCTREEBASENODE_EXTERN_METADATA
#define ENGINE_OCTREEBASENODE_EXTERN_METADATA
	extern template const Engine::Class* const ClassMetaData<class Engine::OctreeBaseNode*>::class_instance;
#endif

#ifndef ENGINE_CAMERA_EXTERN_METADATA
#define ENGINE_CAMERA_EXTERN_METADATA
	extern template const Engine::Class* const ClassMetaData<class Engine::Camera*>::class_instance;
#endif

#ifndef ENGINE_VERTEXBUFFER_EXTERN_METADATA
#define ENGINE_VERTEXBUFFER_EXTERN_METADATA
	extern template const Engine::Class* const ClassMetaData<class Engine::VertexBuffer*>::class_instance;
#endif

#ifndef GAMEINIT_EXTERN_METADATA
#define GAMEINIT_EXTERN_METADATA
	extern template const Engine::Class* const ClassMetaData<class GameInit*>::class_instance;
#endif

#ifndef ENGINE_SKYBOX_EXTERN_METADATA
#define ENGINE_SKYBOX_EXTERN_METADATA
	extern template const Engine::Class* const ClassMetaData<class Engine::Skybox*>::class_instance;
#endif

#ifndef ENGINE_APIOBJECT_EXTERN_METADATA
#define ENGINE_APIOBJECT_EXTERN_METADATA
	extern template const Engine::Class* const ClassMetaData<class Engine::ApiObject*>::class_instance;
#endif

}
