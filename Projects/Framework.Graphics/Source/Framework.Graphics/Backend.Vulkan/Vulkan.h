#pragma once
#include <vulkan/vulkan.h>

namespace W
{
	namespace VK
	{
		const char* TraslateResult(VkResult result);

		VkResult GetSupportedDepthFormat(VkPhysicalDevice physicalDevice, VkFormat& outDepthFormat);
	} // namespace VK
} // namespace W

#define VK_CHECK(result) Debug_AssertMsg(result == VK_SUCCESS, "%s(%d)", ::W::VK::TraslateResult(result), (int)result)
