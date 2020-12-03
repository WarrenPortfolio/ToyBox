#pragma once

#include <vector>
#include <array>
#include <string>
#include <memory>

#include <glm\glm.hpp>
#include <glm\gtx\hash.hpp>

#include <vulkan\vulkan.h>

struct SceneObject
{
	std::string	Name;
};

struct SceneNode : SceneObject
{
	glm::mat4x4 WorldTransform;
	glm::mat4x4 LocalTransform;
};

struct Texture
{
	static std::unique_ptr<Texture> Load(const char* filePath);
	void DestroyPixelBuffer();

	// CPU DataBlock
	int TextureWidth;
	int TextureHeight;
	int TextureChannels;
	void* Pixels;

	// GPU DataBlock
	uint32_t MipLevels;
	VkImage TextureImage;
	VkDeviceMemory TextureImageMemory;
	VkImageView TextureImageView;
	VkSampler TextureSampler;
};

struct Material : SceneObject
{
	// CPU DataBlock
	Texture* DiffuseTexture = nullptr;

	// GPU DataBlock
	VkDescriptorSet DescriptorSets = VK_NULL_HANDLE;
};

struct Vertex
{
	glm::vec3 Position;
	glm::vec3 Color;
	glm::vec2 UV;
	glm::vec3 Normal;
};

struct Mesh
{
	int IndexOffset = 0;
	int TriangleCount = 0;
	int MaterialIndex = 0;
};

struct Model : SceneNode
{
	std::vector<Mesh> Meshs;

	// CPU DataBlock
	std::vector<Vertex> Vertices;
	std::vector<uint32_t> Indices;

	// GPU DataBlock
	VkBuffer VertexBuffer;
	VkDeviceMemory VertexBufferMemory;
	VkBuffer IndexBuffer;
	VkDeviceMemory IndexBufferMemory;
};

struct Camera : SceneNode
{
	float FieldOfView;
};

struct Scene
{
	static std::unique_ptr<Scene> Load(const char* filePath);

	std::vector<std::unique_ptr<Model>> Models;
	std::vector<std::unique_ptr<Material>> Materials;
	std::vector<std::unique_ptr<Texture>> Textures;
	std::vector<std::unique_ptr<Camera>> Cameras;
};