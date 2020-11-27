#include "Scene.h"

#include <Framework.Debug/Debug.h>

#include <fbxsdk.h>
#include <glm/glm.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <stdexcept>

static const int TRIANGLE_VERTEX_COUNT = 3;

//////////////////////////////////////////////////////////////////////////
//                                Texture                               //
//////////////////////////////////////////////////////////////////////////
std::unique_ptr<Texture> Texture::Load(const char* filePath)
{
	int texWidth, texHeight, texChannels;
	stbi_uc* pixels = stbi_load(filePath, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	Debug_AssertMsg(pixels != nullptr, "failed to load texture image!");

	std::unique_ptr<Texture> texture = std::make_unique<Texture>();
	texture->TextureWidth = texWidth;
	texture->TextureHeight = texHeight;
	texture->TextureChannels = texChannels;
	texture->Pixels = pixels;

	return texture;
}

void Texture::DestroyPixelBuffer()
{
	stbi_image_free(Pixels);
	Pixels = nullptr;
}

//////////////////////////////////////////////////////////////////////////
//                        Scene - FBX Converter                         //
//////////////////////////////////////////////////////////////////////////
static void BuildMaterials(Scene& scene, fbxsdk::FbxScene* fbxScene)
{
	int materialCount = fbxScene->GetMaterialCount();
	for (int i = 0; i < materialCount; ++i)
	{
		FbxSurfaceMaterial* fbxMaterial = fbxScene->GetMaterial(i);

		std::unique_ptr<Material> material = std::make_unique<Material>();
		material->Name = fbxMaterial->GetNameOnly();

		const FbxProperty fbxProperty = fbxMaterial->FindProperty(FbxSurfaceMaterial::sDiffuse);
		if (fbxProperty.IsValid())
		{
			const int textureCount = fbxProperty.GetSrcObjectCount<FbxFileTexture>();
			if (textureCount > 0)
			{
				const FbxFileTexture* fbxTexture = fbxProperty.GetSrcObject<FbxFileTexture>();
				if (fbxTexture != nullptr)
				{
					const char* filePath = fbxTexture->GetFileName();

					std::unique_ptr<Texture> texture = Texture::Load(filePath);
					material->DiffuseTexture = texture.get();
					scene.Textures.push_back(std::move(texture));
				}
			}
		}

		scene.Materials.push_back(std::move(material));
	}
}

static void BuildResources(Scene& scene, fbxsdk::FbxScene* fbxScene, fbxsdk::FbxMesh* fbxMesh)
{
	std::unique_ptr<Model> model = std::make_unique<Model>();
	model->Name = fbxMesh->GetNode()->GetNameOnly();

	const int polygonCount = fbxMesh->GetPolygonCount();

	// Count the polygon count of each material
	FbxLayerElementArrayTemplate<int>* materialIndexArray = NULL;
	FbxGeometryElement::EMappingMode materialMappingMode = FbxGeometryElement::eNone;
	if (fbxMesh->GetElementMaterial())
	{
		materialIndexArray = &fbxMesh->GetElementMaterial()->GetIndexArray();
		materialMappingMode = fbxMesh->GetElementMaterial()->GetMappingMode();
		if (materialMappingMode == FbxGeometryElement::eByPolygon)
		{
			if (materialIndexArray->GetCount() == polygonCount)
			{
				// Count the faces of each material
				for (int polygonIndex = 0; polygonIndex < polygonCount; ++polygonIndex)
				{
					const size_t materialIndex = materialIndexArray->GetAt(polygonIndex);
					const size_t requiredMeshSize = materialIndex + 1;
					if (model->Meshs.size() < requiredMeshSize)
					{
						model->Meshs.resize(requiredMeshSize);
					}

					model->Meshs[materialIndex].TriangleCount += 1;
				}
			}
		}
		else if (materialMappingMode == FbxGeometryElement::eAllSame)
		{
			model->Meshs.resize(1);
			model->Meshs[0].TriangleCount = polygonCount * TRIANGLE_VERTEX_COUNT;
		}
	}

	// Initialize the index offset values
	{
		int currentIndexOffset = 0;
		for (Mesh& mesh : model->Meshs)
		{
			mesh.IndexOffset = currentIndexOffset;
			currentIndexOffset += mesh.TriangleCount;

			// reset the triangle count to fill in the index buffer
			mesh.TriangleCount = 0;
		}
	}

	// Populate the index array
	{
		model->Indices.resize(polygonCount * TRIANGLE_VERTEX_COUNT);

		for (int polygonIndex = 0; polygonIndex < polygonCount; ++polygonIndex)
		{
			// The material for current face
			int materiaindex = 0;
			if (materialMappingMode == FbxGeometryElement::eByPolygon)
			{
				materiaindex = materialIndexArray->GetAt(polygonIndex);
			}

			Mesh& mesh = model->Meshs[materiaindex];
			const int polygonIndexOffset = mesh.IndexOffset + (mesh.TriangleCount * TRIANGLE_VERTEX_COUNT);

			for (int vertexIndex = 0; vertexIndex < TRIANGLE_VERTEX_COUNT; ++vertexIndex)
			{
				int controlPointIndex = fbxMesh->GetPolygonVertex(polygonIndex, vertexIndex);
				model->Indices[polygonIndexOffset + vertexIndex] = static_cast<uint32_t>(controlPointIndex);
			}

			mesh.TriangleCount += 1;
		}
	}

	// Populate the index array
	{
		for (int i = 0; i < model->Meshs.size(); ++i)
		{
			FbxSurfaceMaterial* fbxMaterial = fbxMesh->GetNode()->GetMaterial(i);
			int materialCount = fbxScene->GetMaterialCount();
			for (int materialIndex = 0; materialIndex < materialCount; ++materialIndex)
			{
				if (fbxMaterial == fbxScene->GetMaterial(materialIndex))
				{
					model->Meshs[i].MaterialIndex = materialIndex;
				}
			}
		}
	}

	// Populate the vertex array
	{
		const FbxVector4* controlPoints = fbxMesh->GetControlPoints();
		const FbxGeometryElementNormal* normalElement = nullptr;
		const FbxGeometryElementUV* uvElement = nullptr;
		const FbxLayerElementVertexColor* vertexColorSet = nullptr;

		// Normals - only support ByControlPoint mapping
		if (fbxMesh->GetElementNormalCount() > 0)
		{
			FbxGeometryElement::EMappingMode normalMappingMode = fbxMesh->GetElementNormal(0)->GetMappingMode();
			if (normalMappingMode == FbxGeometryElement::eByControlPoint ||
				normalMappingMode == FbxGeometryElement::eByPolygonVertex)
			{
				normalElement = fbxMesh->GetElementNormal(0);
			}
		}

		// UVs - only support ByControlPoint mapping
		if (fbxMesh->GetElementUVCount() > 0)
		{
			FbxGeometryElement::EMappingMode uvMappingMode = fbxMesh->GetElementUV(0)->GetMappingMode();
			if (uvMappingMode == FbxGeometryElement::eByControlPoint ||
				uvMappingMode == FbxGeometryElement::eByPolygonVertex)
			{
				uvElement = fbxMesh->GetElementUV(0);
			}
		}

		if (fbxMesh->GetLayer(0)->GetVertexColors() != nullptr)
		{
			vertexColorSet = fbxMesh->GetLayer(0)->GetVertexColors();
		}

		fbxMesh->GenerateNormals();

		const int controlPointsCount = fbxMesh->GetControlPointsCount();
		model->Vertices.resize(controlPointsCount);

		for (int polygonIndex = 0; polygonIndex < polygonCount; ++polygonIndex)
		{
			for (int vertexIndex = 0; vertexIndex < TRIANGLE_VERTEX_COUNT; ++vertexIndex)
			{
				int polygonVertexIndex = (polygonIndex * TRIANGLE_VERTEX_COUNT) + vertexIndex;
				int index = fbxMesh->GetPolygonVertex(polygonIndex, vertexIndex);

				Vertex& vertex = model->Vertices[index];

				// Save the vertex position
				vertex.Position = glm::vec3(
					static_cast<float>(controlPoints[index][0]),
					static_cast<float>(controlPoints[index][1]),
					static_cast<float>(controlPoints[index][2])
				);

				FbxVector4 vertexNormal;
				if (fbxMesh->GetPolygonVertexNormal(polygonIndex, vertexIndex, vertexNormal))
				{
					vertex.Normal = glm::vec3(vertexNormal[0], vertexNormal[1], vertexNormal[2]);
				}
				else
				{
					Debug_AssertMsg(false, "failed to get normal");
				}

				// Save the UV
				if (uvElement != nullptr)
				{
					int uvIndex = (uvElement->GetMappingMode() == FbxGeometryElement::eByControlPoint)
						? index
						: polygonVertexIndex;

					if (uvElement->GetReferenceMode() == FbxLayerElement::eIndexToDirect)
					{
						uvIndex = uvElement->GetIndexArray().GetAt(uvIndex);
					}

					FbxVector2 uv = uvElement->GetDirectArray().GetAt(uvIndex);

					vertex.UV = glm::vec2(
						static_cast<float>(uv[0]),
						1.0f - static_cast<float>(uv[1])
					);
				}

				if (vertexColorSet != nullptr)
				{
					int colorIndex = index;
					if (vertexColorSet->GetReferenceMode() == FbxLayerElement::eIndexToDirect)
					{
						colorIndex = vertexColorSet->GetIndexArray().GetAt(index);
					}

					FbxColor color = vertexColorSet->GetDirectArray().GetAt(colorIndex);

					vertex.Color = glm::vec3(
						static_cast<float>(color[0]),
						static_cast<float>(color[1]),
						static_cast<float>(color[2])
					);
				}
				else
				{
					vertex.Color = glm::vec3(1.0f, 1.0f, 1.0f);
				}
			}
		}
	}

	scene.Models.push_back(std::move(model));
}

static void BuildResources(Scene& scene, fbxsdk::FbxScene* fbxScene, FbxNode* fbxNode)
{
	FbxNodeAttribute* nodeAttribute = fbxNode->GetNodeAttribute();
	if (nodeAttribute != nullptr)
	{
		if (nodeAttribute->GetAttributeType() == FbxNodeAttribute::eMesh)
		{
			FbxMesh* fbxMesh = fbxNode->GetMesh();
			if (fbxMesh != nullptr)
			{
				BuildResources(scene, fbxScene, fbxMesh);
			}
		}
	}

	const int childCount = fbxNode->GetChildCount();
	for (int childIndex = 0; childIndex < childCount; ++childIndex)
	{
		BuildResources(scene, fbxScene, fbxNode->GetChild(childIndex));
	}
}

//////////////////////////////////////////////////////////////////////////
//                                Scene                                 //
//////////////////////////////////////////////////////////////////////////
std::unique_ptr<Scene> Scene::Load(const char* filePath)
{
	// The first thing to do is to create the FBX Manager which is the object allocator for almost all the classes in the SDK
	FbxManager* fbxManager = FbxManager::Create();

	// Create an IOSettings object. This object holds all import/export settings.
	FbxIOSettings* ioSettings = FbxIOSettings::Create(fbxManager, IOSROOT);
	fbxManager->SetIOSettings(ioSettings);

	// Create an FBX scene. This object holds most objects imported/exported from/to files.
	FbxScene* fbxScene = FbxScene::Create(fbxManager, "Scene");

	// Create the importer.
	FbxImporter* importer = FbxImporter::Create(fbxManager, "LoadScene");

	// Initialize the importer by providing a filename.
	std::unique_ptr<Scene> scene;
	if (importer->Initialize(filePath) == true)
	{
		if (importer->Import(fbxScene) == true)
		{
			// Convert Axis System to desired (Blender), if needed
			FbxAxisSystem sceneAxisSystem = fbxScene->GetGlobalSettings().GetAxisSystem();
			FbxAxisSystem desiredAxisSystem(FbxAxisSystem::eZAxis, FbxAxisSystem::eParityEven, FbxAxisSystem::eRightHanded);
			if (sceneAxisSystem != desiredAxisSystem)
			{
				desiredAxisSystem.ConvertScene(fbxScene);
			}

			// Convert Unit System to desired, if needed
			FbxSystemUnit sceneSystemUnit = fbxScene->GetGlobalSettings().GetSystemUnit();
			FbxSystemUnit desiredSystemUnit = FbxSystemUnit::m;
			if (sceneSystemUnit != desiredSystemUnit)
			{
				desiredSystemUnit.ConvertScene(fbxScene);
			}

			// Convert mesh, NURBS and patch into triangle mesh
			FbxGeometryConverter geomConverter(fbxManager);
			geomConverter.Triangulate(fbxScene, /*replace*/true);

			// Create scene
			scene = std::make_unique<Scene>();

			// Build the graphics resources
			BuildMaterials(*scene, fbxScene);
			BuildResources(*scene, fbxScene, fbxScene->GetRootNode());
		}
		else
		{
			Debug_AssertMsg(false, "failed to import scene");
		}
	}
	else
	{
		Debug_AssertMsg(false, "failed to initialize impoter");
	}

	// Destroy the importer to release the file.
	importer->Destroy();
	importer = nullptr;

	// Destroy the manager.
	fbxManager->Destroy();
	fbxManager = nullptr;

	return scene;
}