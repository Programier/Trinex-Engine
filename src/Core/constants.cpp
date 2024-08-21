#include <Core/constants.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/logger.hpp>
#include <Core/memory.hpp>
#include <thread>

namespace Engine
{
	const uint_t Constants::processor_count = std::thread::hardware_concurrency();
	const Vector3D Constants::OX(1.f, 0.f, 0.f);
	const Vector3D Constants::OY(0.f, 1.f, 0.f);
	const Vector3D Constants::OZ(0.f, 0.f, 1.f);
	const Quaternion Constants::zero_rotation({0.f, 0.f, 0.f});
	const Vector3D Constants::right_vector(1.f, 0.f, 0.f);
	const Vector3D Constants::up_vector(0.f, 1.f, 0.f);
	const Vector3D Constants::forward_vector(0.f, 0.f, -1.f);
	const float Constants::PI                             = glm::pi<float>();
	const float Constants::E                              = glm::e<float>();
	const glm::mat4 Constants::identity_matrix            = Matrix4f(1.0f);
	const glm::mat4 Constants::zero_matrix                = Matrix4f();
	const glm::vec4 Constants::identity_vector            = Vector4D(1.f, 1.f, 1.f, 1.f);
	const glm::vec4 Constants::zero_vector                = Vector4D(0.f, 0.f, 0.f, 0.f);
	const float Constants::min_positive_float             = 0.000001f;
	const Vector3D Constants::min_positive_vector         = Vector3D(0.000001f);
	const ArrayIndex Constants::index_none                = ~static_cast<ArrayIndex>(0);
	const ArrayOffset Constants::offset_none              = ~static_cast<ArrayOffset>(0);
	const HashIndex Constants::invalid_hash               = static_cast<HashIndex>(0);
	const size_t Constants::max_size                      = ~static_cast<size_t>(0);
	const IntVector4D Constants::int_zero_vector          = IntVector4D(0);
	const IntVector4D Constants::int_identity_vector      = IntVector4D(1);
	const IntVector4D Constants::uint_zero_vector         = UIntVector4D(0);
	const IntVector4D Constants::uint_identity_vector     = UIntVector4D(1);
	const String Constants::asset_extention               = ".tasset";
	const String Constants::shader_extention              = ".tsdr";
	const String Constants::name_separator                = "::";
	const PriorityIndex Constants::max_priority           = ~static_cast<PriorityIndex>(0);
	const String Constants::default_entry_point           = "Engine::EngineStart";
	const String Constants::library_load_function_name    = "create_library_interface";
	const String Constants::script_extension              = ".as";
	const String Constants::script_byte_code_extension    = ".asbc";
	const String Constants::virtual_file_system_extension = ".tvfs";
	const String Constants::translation_config_extension  = ".lang";
	const ByteColor3 Constants::splash_text_color         = {255, 255, 255};
	const HashIndex Constants::script_userdata_id = memory_hash_fast(reinterpret_cast<const void*>("script_userdata_id"), 18, 0);
}// namespace Engine
