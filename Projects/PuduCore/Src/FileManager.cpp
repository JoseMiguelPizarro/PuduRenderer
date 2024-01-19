#pragma once
#include <filesystem>
#include <fmt/core.h>

#include <fastgltf/parser.hpp>
#include <fastgltf/types.hpp>
#include <fastgltf/tools.hpp>
#include <fastgltf/glm_element_traits.hpp>

#include "FileManager.h"
#include "Logger.h"
#include "PuduGraphics.h"

#ifndef TINYOBJLOADER_IMPLEMENTATION
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#endif
#include <MaterialCreationData.h>
#include <EntityManager.h>


namespace fs = std::filesystem;

namespace Pudu {
	std::vector<char> FileManager::ReadFile(std::filesystem::path const& fileName)
	{
		Print("Current file");
		Print(fs::current_path().string().c_str());

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
		auto path = std::filesystem::path(ASSETS_FOLDER_PATH);
		path.append(fileName);
		auto absPath = std::filesystem::absolute(path);
		//auto path = std::format("{}/{}", ASSETS_FOLDER_PATH, fileName);

		return ReadFile(absPath);
	}

	std::vector<char> FileManager::ReadShaderFile(const std::string& shaderPath)
	{
		auto path = GetAssetPath(shaderPath).string();
		//	path = std::format("{}/{}.spv", path, "Compiled");

		return std::vector<char>();

		//return ReadFile(path);
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
			PUDU_ERROR(warn + err);
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

				vertex.color = { 1.0f, 1.0f, 1.0f };

				vertices.push_back(vertex);
				indices.push_back(static_cast<uint32_t>(indices.size()));
			}
		}

		MeshCreationData data;
		data.Vertices = vertices;
		data.Indices = indices;

		return data;
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
			LOG("{}",node.name);

			auto name = std::string(node.name);
			auto meshIndex = node.meshIndex;
			if (meshIndex.has_value())
			{
				auto meshData = meshCreationData[meshIndex.value()];
				auto model = PuduGraphics::Instance()->CreateModel(meshData);

				RenderEntitySPtr e = EntityManager::AllocateRenderEntity(name, model);
				entities.push_back(e);
			}
			else {
				EntitySPtr entity = EntityManager::AllocateEntity(name);
				fg::TRS transform = std::get<fg::TRS>(node.transform);

				glm::quat r = glm::quat(transform.rotation[3], transform.rotation[0], transform.rotation[1], transform.rotation[2]);

				Transform& t = entity->GetTransform();
				t.SetRotation(r);
				t.LocalPosition = vec3(transform.translation[0], transform.translation[1], transform.translation[2]);
				t.LocalScale = vec3(transform.scale[0], transform.scale[1], transform.scale[2]);

				entities.push_back(entity);

				for (auto child : node.children) {
					entities[child]->SetParent(entity);
				}
			}
		}

		return entities;
	}

	GltfAsset FileManager::LoadGltfAsset(fs::path const& path)
	{
		auto pathAssetFolder = FileManager::GetAssetPath(path.string());

		constexpr auto gltfOptions =
			fastgltf::Options::DontRequireValidAssetMember |
			fastgltf::Options::AllowDouble |
			fastgltf::Options::LoadGLBBuffers |
			fastgltf::Options::LoadExternalBuffers |
			fastgltf::Options::GenerateMeshIndices;

		fastgltf::Parser parser;
		fastgltf::GltfDataBuffer data;
		data.loadFromFile(pathAssetFolder);
		GltfAsset asset = parser.loadGltf(&data, pathAssetFolder.parent_path(), gltfOptions);

		if (auto error = asset.error() != fastgltf::Error::None)
		{
			Print(std::to_string(error).c_str());
			PUDU_ERROR("Invalid gltf asset");
		}

		return asset;
	}

	std::vector<MeshCreationData> FileManager::GetGltfMeshCreationData(fs::path const& path, GltfAsset& gltfAsset)
	{
		std::vector<MeshCreationData> creationData;

		for (auto& mesh : gltfAsset->meshes) {
			LOG("Processing mesh {} primitives: {}\n", mesh.name, mesh.primitives.size());
			for (auto primitive : mesh.primitives) {
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

				auto& positionsAccessor = gltfAsset->accessors[primitive.findAttribute("POSITION")->second];
				vertices.resize(positionsAccessor.count);


				for (auto& attribute : primitive.attributes) {

					auto accessor = gltfAsset->accessors[attribute.second];
					const char* attribName = attribute.first.c_str();
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
				}

				auto& gltfMat = gltfAsset->materials[primitive.materialIndex.value()];

				auto baseTextSource = gltfAsset->images[gltfMat.pbrData.baseColorTexture.value().textureIndex].data;

				auto uri = std::get_if<fastgltf::sources::URI>(&baseTextSource);

				auto baseTexturePath = fs::path(uri->uri.path());

				LOG("Base texture path {}", baseTexturePath.string());

				MeshCreationData meshCreationData;
				MaterialCreationData materialCreationData;
				materialCreationData.BasetTexturePath = path.parent_path().relative_path() / baseTexturePath;
				meshCreationData.Indices = indices;
				meshCreationData.Vertices = vertices;
				meshCreationData.Material = materialCreationData;
				meshCreationData.Name = mesh.name;
				creationData.push_back(meshCreationData);
			}
		}

		return creationData;
	}

}

