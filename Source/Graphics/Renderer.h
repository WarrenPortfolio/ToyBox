#pragma once

#include <glm/glm.hpp>

#include <vulkan/vulkan.h>

#include <unordered_map>
#include <memory>

struct Texture;
struct Model;
struct Material;
struct Scene;

struct GLFWwindow;

struct QueueFamilyIndices
{
	int GraphicsFamily = -1;
	int PresentFamily = -1;

	bool IsComplete()
	{
		return GraphicsFamily >= 0 && PresentFamily >= 0;
	}
};

struct SwapChainSupportDetails
{
	VkSurfaceCapabilitiesKHR Capabilities;
	std::vector<VkSurfaceFormatKHR> Formats;
	std::vector<VkPresentModeKHR> PresentModes;
};

struct UniformBufferObject
{
	glm::mat4 Model;
	glm::mat4 View;
	glm::mat4 Projection;
	glm::vec3 CameraPosition;
	float     AmbientLightIntensity;
	glm::vec3 AmbientLightColor;
};

class Renderer
{
public:
	Renderer();
	~Renderer();

	void Startup();
	void Shutdown();

	void DrawFrame();

private:
	GLFWwindow* mWindow = nullptr;

	VkDebugReportCallbackEXT mCallbackExt;

	VkInstance mInstance;
	VkSurfaceKHR mSurface;

	VkPhysicalDevice mPhysicalDevice = VK_NULL_HANDLE;
	VkDevice mDevice;

	VkQueue mGraphicsQueue;
	VkQueue mPresentQueue;

	VkSwapchainKHR mSwapChain;
	std::vector<VkImage> mSwapChainImages;
	VkFormat mSwapChainImageFormat;
	VkExtent2D mSwapChainExtent;
	std::vector<VkImageView> mSwapChainImageViews;
	std::vector<VkFramebuffer> mSwapChainFramebuffers;

	VkRenderPass mRenderPass;
	VkDescriptorSetLayout mDescriptorSetLayout;
	VkDescriptorSetLayout mDescriptorSetLayout2;
	VkPipelineLayout mPipelineLayout;
	VkPipeline mGraphicsPipeline;

	VkCommandPool mCommandPool;

	VkImage mDepthImage;
	VkDeviceMemory mDepthImageMemory;
	VkImageView mDepthImageView;

	std::unique_ptr<Scene> mScene;

	std::vector<VkBuffer> mUniformBuffers;
	std::vector<VkDeviceMemory> mUniformBuffersMemory;

	VkDescriptorPool mDescriptorPool;

	std::vector<VkCommandBuffer> mCommandBuffers;

	size_t mCurrentFrame = 0;

	struct FrameData
	{
		VkCommandBuffer     CommandBuffer;
		VkFence             Fence;
		VkSemaphore         ImageAcquiredSemaphore;
		VkSemaphore         RenderCompleteSemaphore;
	};

	std::vector<FrameData> mFrameData;

	VkAllocationCallbacks mAllocationCallbacks;

	bool mFrameBufferResized = false;

private:
	static void FramebufferResizeCallback(GLFWwindow* window, int width, int height);

	void InitRenderDoc();
	void InitWindow();
	void InitVulkan();
	void InitImGui();

	void SetupDebugCallback();

	void CleanupSwapChain();
	void RecreateSwapChain();

	void CreateInstance();
	void CreateSurface();

	void PickPhysicalDevice();
	void CreateLogicalDevice();

	void CreateSwapChain();
	void CreateFrameData();
	void CreateImageViews();
	void CreateRenderPass();
	void CreateDescriptorSetLayout();
	void CreateGraphicsPipeline();
	void CreateFramebuffers();
	void CreateCommandPool();
	void CreateDepthResources();

	VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
	VkFormat FindDepthFormat();

	bool HasStencilComponent(VkFormat format);

	void CreateTextureImage(Texture* texture);
	void CreateMaterial(Material* material);

	void GenerateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);

	VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);
	void CreateImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
	void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);
	void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

	void LoadScene();

	void CreateVertexBuffer(Model* model);
	void CreateIndexBuffer(Model* model);

	void CreateUniformBuffers();
	void CreateDescriptorPool();
	void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

	VkCommandBuffer BeginSingleTimeCommands();
	void EndSingleTimeCommands(VkCommandBuffer commandBuffer);

	void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	void CreateCommandBuffers();

	void UpdateUniformBuffer(uint32_t currentImage);

	VkShaderModule CreateShaderModule(const std::vector<char>& code);
	VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes);
	VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
	SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);

	bool IsDeviceSuitable(VkPhysicalDevice device);
	bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
	QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);
	std::vector<const char*> GetRequiredExtensions();
	bool CheckValidationLayerSupport();
	uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
};


