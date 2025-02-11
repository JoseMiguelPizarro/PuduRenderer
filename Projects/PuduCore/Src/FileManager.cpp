#pragma once
#include "FileManager.h"

#include <filesystem>



#include "Logger.h"
#include "PuduGraphics.h"

#ifndef TINYOBJLOADER_IMPLEMENTATION
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#endif
#include <MaterialCreationData.h>
#include <EntityManager.h>

#include <fastgltf/core.hpp>
#include <fastgltf/types.hpp>
#include <fastgltf/tools.hpp>
#include <fastgltf/util.hpp>
#include <fastgltf/glm_element_traits.hpp>

namespace fs = std::filesystem;
namespace fg = fastgltf;


namespace Pudu {


	std::vector<char> FileManager::ReadFile(std::filesystem::path const& fileName)
	{
		std::ifstream file(fileName.generic_string(), std::ios::ate | std::ios::binary); //ate: at the end lmao

		if (!file.is_open())
		{
			Print(fileName.string().c_str());
			throw std::runtime_error("Failed to open file");
		}

		size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);

		file.seekg(0); //Go back to the beggining of the file
		file.read(buffer.data(), fileSize);

		file.close();

		return buffer;
	}

	/// <summary>
	/// Get path relative to assets folder
	/// </summary>
	inline fs::path FileManager::GetAssetPath(fs::path const& path)
	{
		return fs::absolute(ASSETS_FOLDER_PATH / path);
	}

	/// <summary>
	/// Read asset relative to the assets folder
	/// </summary>
	std::vector<char> FileManager::ReadAssetFile(const std::string& fileName)
	{
		auto path = fs::path(ASSETS_FOLDER_PATH);
		path.append(fileName);
		auto absPath = fs::absolute(path);
		//auto path = std::format("{}/{}", ASSETS_FOLDER_PATH, fileName);

		return ReadFile(absPath);
	}

	std::vector<char> FileManager::ReadAssetFile(const fs::path& path)
	{
		auto root = fs::path(ASSETS_FOLDER_PATH);
		root = root / path;

		return ReadFile(fs::absolute(root));
	}

	MeshCreationData FileManager::LoadModelObj(std::string const& assetPath)
	{
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warn, err;
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;

		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, GetAssetPath(assetPath).string().c_str()))
		{
			LOG_ERROR("{} {}", warn, err);
		}
		for (auto& shape : shapes)
		{
			for (const auto& index : shape.mesh.indices)
			{
				Vertex vertex{};

				vertex.pos = {
					attrib.vertices[3 * index.vertex_index + 0],
					attrib.vertices[3 * index.vertex_index + 1],
					attrib.vertices[3 * index.vertex_index + 2],
				};

				vertex.texcoord = {
					attrib.texcoords[2 * index.texcoord_index + 0],
					1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
					//Obj assumes bottom left corner to be 0. vulkan upload images from (0,-1)
				};

				vertex.color =
					{ attrib.colors[3 * index.vertex_index + 0], attrib.colors[3 * index.vertex_index + 1], attrib.colors[3 * index.vertex_index + 2] };

				vertices.push_back(vertex);
				indices.push_back(static_cast<uint32_t>(indices.size()));
			}
		}

		MeshCreationData data;
		data.Vertices = vertices;
		data.Indices = indices;

		return data;
	}

	fs::path GetPathFromTextureIndex(fg::Expected<fastgltf::Asset>& asset, uint16_t index, fs::path const& path) {

		auto baseTextSource = asset->images[index].data;
		auto uri = std::get_if<fastgltf::sources::URI>(&baseTextSource);
		auto baseTexturePath = fs::path(uri->uri.path());
		LOG("Base texture path {}", baseTexturePath.string());
		return path.parent_path().relative_path() / baseTexturePath;
	}

	fg::Expected<fastgltf::Asset> LoadGltfAsset(fs::path const& path)
	{
		auto pathAssetFolder = FileManager::GetAssetPath(path.string());

		constexpr auto gltfOptions =
			fastgltf::Options::DontRequireValidAssetMember |
			fastgltf::Options::AllowDouble |
			fastgltf::Options::LoadGLBBuffers |
			fastgltf::Options::LoadExternalBuffers |
			fastgltf::Options::GenerateMeshIndices;

		fastgltf::Parser parser;
		//data.FromPath(pathAssetFolder);

		auto gltfFile = fastgltf::MappedGltfFile::FromPath(pathAssetFolder);

		fg::Expected<fastgltf::Asset> asset = parser.loadGltf(gltfFile.get(), pathAssetFolder.parent_path(), gltfOptions);

		if (auto error = asset.error() != fastgltf::Error::None)
		{
			Print(std::to_string(error).c_str());
			LOG_ERROR("Invalid gltf asset");
		}

		return asset;
	}

	std::vector<MeshCreationData> GetGltfMeshCreationData(fs::path const& path, fg::Expected<fastgltf::Asset>& gltfAsset)
	{
		std::vector<MeshCreationData> creationData;

		for (auto& mesh : gltfAsset->meshes) {
			LOG("Processing mesh {} primitives: {}\n", mesh.name, mesh.primitives.size());
			for (auto& primitive : mesh.primitives) {
				if (!primitive.indicesAccessor.has_value())
				{
					continue;
				}

				std::vector<Vertex> vertices;
				std::vector<uint32_t> indices;
				fastgltf::Accessor& indexAccessor = gltfAsset->accessors[primitive.indicesAccessor.value()];
				indices.resize(indexAccessor.count);

				fastgltf::iterateAccessorWithIndex<uint32_t>(gltfAsset.get(), indexAccessor, [&](uint32_t index, size_t i) {
					indices[i] = index;
				});

				auto& positionsAccessor = gltfAsset->accessors[primitive.findAttribute("POSITION")->accessorIndex];
				vertices.resize(positionsAccessor.count);

				for (auto& attribute : primitive.attributes) {

					auto accessor = gltfAsset->accessors[attribute.accessorIndex];
					const char* attribName = attribute.name.c_str();
					if (strcmp(attribName, "POSITION") == 0)
					{
						std::size_t idx = 0;
						fastgltf::iterateAccessor<vec3>(gltfAsset.get(), accessor, [&](vec3 v) {
							vertices[idx++].pos = v;
						});
					}

					if (strcmp(attribName, "TEXCOORD_0") == 0)
					{
						std::size_t idx = 0;
						fastgltf::iterateAccessor<vec2>(gltfAsset.get(), accessor, [&](vec2 v) {
							vertices[idx++].texcoord = v;
						});
					}

					if (strcmp(attribName, "NORMAL") == 0)
					{
						std::size_t idx = 0;
						fastgltf::iterateAccessor<vec3>(gltfAsset.get(), accessor, [&](vec3 v) {
							vertices[idx++].normal = v;
						});
					}
					if (strcmp(attribName, "COLOR_0") == 0)
					{
						std::size_t idx = 0;
						fastgltf::iterateAccessor<vec3>(gltfAsset.get(), accessor, [&](vec3 v) {
							vertices[idx++].color = v;
						});
					}
				}

				MeshCreationData meshCreationData{};
				MaterialCreationData materialCreationData{};

				if (bool hasMat = !gltfAsset->materials.empty())
				{
					auto& gltfMat = gltfAsset->materials[primitive.materialIndex.value()];

					bool hasBaseTexture, hasNormalMap = false;

					hasBaseTexture = gltfMat.pbrData.baseColorTexture.has_value();
					hasNormalMap = gltfMat.normalTexture.has_value();


					if (hasBaseTexture)
					{
						materialCreationData.BaseTexturePath = GetPathFromTextureIndex(gltfAsset, gltfMat.pbrData.baseColorTexture.value().textureIndex, path);
					}
					if (hasNormalMap)
					{
						materialCreationData.NormalMapPath = GetPathFromTextureIndex(gltfAsset, gltfMat.normalTexture.value().textureIndex, path);
					}

					materialCreationData.hasBaseTexture = hasBaseTexture;
					materialCreationData.hasNormalMap = hasNormalMap;
				}

				meshCreationData.Indices = indices;
				meshCreationData.Vertices = vertices;
				meshCreationData.Material = materialCreationData;
				meshCreationData.Name = mesh.name;
				creationData.push_back(meshCreationData);
			}
		}

		return creationData;
	}

	std::vector<MeshCreationData> FileManager::LoadModelGltf(fs::path const& path)
	{
		auto asset = LoadGltfAsset(path);

		return GetGltfMeshCreationData(path, asset);
	}

	std::vector<EntitySPtr> FileManager::LoadGltfScene(fs::path const& path)
	{
		auto asset = LoadGltfAsset(path);

		auto meshCreationData = GetGltfMeshCreationData(path, asset);

		std::vector<EntitySPtr> entities;

		for (auto node : asset->nodes) {
			LOG("{}", node.name);

			auto name = std::string(node.name);
			auto meshIndex = node.meshIndex;

			EntitySPtr entity;
			bool shouldUpdateTransforms = false;
			//Node is a Model
			if (meshIndex.has_value())
			{
				const auto& meshData = meshCreationData[meshIndex.value()];
				auto model = PuduGraphics::Instance()->CreateModel(meshData);

				auto renderEntity = EntityManager::AllocateRenderEntity(name, model);
				auto [layer] = renderEntity->GetRenderSettings();
				layer = 0;
				entity = renderEntity;
			}
			//Node is a transform
			else {
				entity = EntityManager::AllocateEntity(name);

				shouldUpdateTransforms = true;
			}

			entities.push_back(entity);

			//Set node transform
			auto [translation, rotation, scale] = std::get<fg::TRS>(node.transform);
			auto r = glm::quat(rotation[3], rotation[0], rotation[1], rotation[2]);

			Transform& t = entity->GetTransform();
			t.SetRotation(r);
			t.SetLocalPosition(vec3(translation[0], translation[1], translation[2]));
			t.SetLocalScale(vec3(scale[0], scale[1], scale[2]));

			if (shouldUpdateTransforms)
			{
				for (auto child : node.children) {
					entities[child]->SetParent(entity);
				}
			}
		}

		return entities;
	}

	std::vector<char> FileManager::LoadShader(fs::path const& path)
	{
		auto compiledShaderPath = path.parent_path() / "Compiled" / path.filename().concat(".spv");
		return ReadAssetFile(compiledShaderPath);
	}

	std::vector<glm::vec4> FileManager::LoadPointCloud(fs::path const& path)
	{
		auto assetPath = GetAssetPath(path);
		std::vector<glm::vec4> pointCloud;  // List to store the points
		std::ifstream file(assetPath);   // Open the file

		if (!file.is_open()) {
			LOG_ERROR("Failed to open file {}", path.string().c_str());
			return pointCloud;
		}

		std::string line;
		while (std::getline(file, line)) {
			std::istringstream iss(line);
			float x, y, z;
			if (iss >> x >> y >> z) {
				pointCloud.push_back(glm::vec4(x, y, z,0));  // Store the point in the list
			}
		}

		file.close();  // Close the file
		return pointCloud;
	}






}

