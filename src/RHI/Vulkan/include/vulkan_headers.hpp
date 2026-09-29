#pragma once

#define VK_NO_PROTOTYPES 1
#define VULKAN_HPP_NO_EXCEPTIONS 1
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#define VULKAN_HPP_ASSERT trinex_assert
#define VULKAN_HPP_ASSERT_ON_RESULT trinex_assert
#define VULKAN_HPP_NO_SMART_HANDLE 1
#define VULKAN_HPP_NO_TO_STRING 1
#define VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS 1

#include <vulkan/vulkan.hpp>


namespace VULKAN_HPP_NAMESPACE
{
#if TRINEX_ENABLE_ASSERTS
	VULKAN_HPP_INLINE VULKAN_HPP_CONSTEXPR_20 const char* to_c_string(Result value)
	{
		switch (value)
		{
			case Result::eSuccess: return "Success";
			case Result::eNotReady: return "NotReady";
			case Result::eTimeout: return "Timeout";
			case Result::eEventSet: return "EventSet";
			case Result::eEventReset: return "EventReset";
			case Result::eIncomplete: return "Incomplete";
			case Result::eErrorOutOfHostMemory: return "ErrorOutOfHostMemory";
			case Result::eErrorOutOfDeviceMemory: return "ErrorOutOfDeviceMemory";
			case Result::eErrorInitializationFailed: return "ErrorInitializationFailed";
			case Result::eErrorDeviceLost: return "ErrorDeviceLost";
			case Result::eErrorMemoryMapFailed: return "ErrorMemoryMapFailed";
			case Result::eErrorLayerNotPresent: return "ErrorLayerNotPresent";
			case Result::eErrorExtensionNotPresent: return "ErrorExtensionNotPresent";
			case Result::eErrorFeatureNotPresent: return "ErrorFeatureNotPresent";
			case Result::eErrorIncompatibleDriver: return "ErrorIncompatibleDriver";
			case Result::eErrorTooManyObjects: return "ErrorTooManyObjects";
			case Result::eErrorFormatNotSupported: return "ErrorFormatNotSupported";
			case Result::eErrorFragmentedPool: return "ErrorFragmentedPool";
			case Result::eErrorUnknown: return "ErrorUnknown";
			case Result::eErrorValidationFailed: return "ErrorValidationFailed";
			case Result::eErrorOutOfPoolMemory: return "ErrorOutOfPoolMemory";
			case Result::eErrorInvalidExternalHandle: return "ErrorInvalidExternalHandle";
			case Result::eErrorInvalidOpaqueCaptureAddress: return "ErrorInvalidOpaqueCaptureAddress";
			case Result::eErrorFragmentation: return "ErrorFragmentation";
			case Result::ePipelineCompileRequired: return "PipelineCompileRequired";
			case Result::eErrorNotPermitted: return "ErrorNotPermitted";
			case Result::eErrorSurfaceLostKHR: return "ErrorSurfaceLostKHR";
			case Result::eErrorNativeWindowInUseKHR: return "ErrorNativeWindowInUseKHR";
			case Result::eSuboptimalKHR: return "SuboptimalKHR";
			case Result::eErrorOutOfDateKHR: return "ErrorOutOfDateKHR";
			case Result::eErrorIncompatibleDisplayKHR: return "ErrorIncompatibleDisplayKHR";
			case Result::eErrorInvalidShaderNV: return "ErrorInvalidShaderNV";
			case Result::eErrorImageUsageNotSupportedKHR: return "ErrorImageUsageNotSupportedKHR";
			case Result::eErrorVideoPictureLayoutNotSupportedKHR: return "ErrorVideoPictureLayoutNotSupportedKHR";
			case Result::eErrorVideoProfileOperationNotSupportedKHR: return "ErrorVideoProfileOperationNotSupportedKHR";
			case Result::eErrorVideoProfileFormatNotSupportedKHR: return "ErrorVideoProfileFormatNotSupportedKHR";
			case Result::eErrorVideoProfileCodecNotSupportedKHR: return "ErrorVideoProfileCodecNotSupportedKHR";
			case Result::eErrorVideoStdVersionNotSupportedKHR: return "ErrorVideoStdVersionNotSupportedKHR";
			case Result::eErrorInvalidDrmFormatModifierPlaneLayoutEXT: return "ErrorInvalidDrmFormatModifierPlaneLayoutEXT";
			case Result::eErrorPresentTimingQueueFullEXT: return "ErrorPresentTimingQueueFullEXT";
#if defined(VK_USE_PLATFORM_WIN32_KHR)
			case Result::eErrorFullScreenExclusiveModeLostEXT: return "ErrorFullScreenExclusiveModeLostEXT";
#endif
			case Result::eThreadIdleKHR: return "ThreadIdleKHR";
			case Result::eThreadDoneKHR: return "ThreadDoneKHR";
			case Result::eOperationDeferredKHR: return "OperationDeferredKHR";
			case Result::eOperationNotDeferredKHR: return "OperationNotDeferredKHR";
			case Result::eErrorInvalidVideoStdParametersKHR: return "ErrorInvalidVideoStdParametersKHR";
			case Result::eErrorCompressionExhaustedEXT: return "ErrorCompressionExhaustedEXT";
			case Result::eIncompatibleShaderBinaryEXT: return "IncompatibleShaderBinaryEXT";
			case Result::ePipelineBinaryMissingKHR: return "PipelineBinaryMissingKHR";
			case Result::eErrorNotEnoughSpaceKHR: return "ErrorNotEnoughSpaceKHR";
			default: return "Undefined";
		}
	}

#endif

	inline void check_result(vk::Result result)
	{
		trinex_assert_msg(result == vk::Result::eSuccess, to_c_string(result));
	}

	template<typename T>
	inline T& check_result(vk::ResultValue<T>& result)
	{
		check_result(result.result);
		return result.value;
	}

	template<typename T>
	inline T check_result(vk::ResultValue<T>&& result)
	{
		check_result(result.result);
		return std::move(result.value);
	}
}// namespace VULKAN_HPP_NAMESPACE
