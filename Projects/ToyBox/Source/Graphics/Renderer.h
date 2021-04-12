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
	struct Light
	{
		alignas(16) glm::vec3	Position;
		alignas(4)  int			Type;

		alignas(16) glm::vec3	Direction;
		alignas(4)  float		Range;

		alignas(16) glm::vec3	Color;
		alignas(4)  float		Intensity;

		alignas(4)  float		InnerAngle;
		alignas(4)  float		OuterAngle;
	};

	alignas(64) glm::mat4	View;
	alignas(64) glm::mat4	Projection;
	alignas(16) glm::vec3	CameraPosition;

	alignas(4)  float		AmbientLightIntensity;
	alignas(16) glm::vec3	AmbientLightColor;

	alignas(4)  float		DirectionalLightIntensity;
	alignas(16) glm::vec3	DirectionalLightColor;
	alignas(16) glm::vec3	DirectionalLightDirection;

	alignas(16) glm::vec3	MaterialColor;
	alignas(16) glm::vec3	MaterialSpecularColor;
	alignas(4)  float		MaterialRoughness;

	alignas(4)  int			LightCount;
	alignas(16) Light		Lights[8];
};

struct UniformPushConstant
{
	alignas(64) glm::mat4	Model;
};

class Renderer
{
public:
	Renderer();
	~Renderer();

	void Startup();
	void Shutdown();

	void FrameUpdate(float deltaTime);
	void FrameRender();
	void FramePresent();

private:

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

	VkBuffer mUniformBuffers;
	VkDeviceMemory mUniformBuffersMemory;

	VkDescriptorPool mDescriptorPool;

	uint32_t mCurrentFrame = 0;
	uint32_t imageIndex = 0;

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

	void UpdateUniformBuffer(VkCommandBuffer commandBuffer);

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


