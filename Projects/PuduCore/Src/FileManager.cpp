#pragma once
#include <filesystem>
#include <FileManager.h>
#include "Logger.h"

#ifndef TINYOBJLOADER_IMPLEMENTATION
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#endif

namespace fs = std::filesystem;

namespace Pudu {
	std::vector<char> FileManager::ReadFile(const std::filesystem::path fileName)
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
	inline fs::path FileManager::GetAssetPath(std::string path)
	{
		fs::path p = fs::path(ASSETS_FOLDER_PATH);
		p.append(path);

		return p;
	}

	/// <summary>
	/// Read asset relative to the assets folder
	/// </summary>
	std::vector<char> FileManager::ReadAssetFile(const std::string& fileName)
	{
		auto path = std::filesystem::path(ASSETS_FOLDER_PATH);
		path.append(fileName);
		//auto path = std::format("{}/{}", ASSETS_FOLDER_PATH, fileName);

		return ReadFile(path);
	}

	std::vector<char> FileManager::ReadShaderFile(const std::string& shaderPath)
	{
		auto path = GetAssetPath(shaderPath).string();
		//	path = std::format("{}/{}.spv", path, "Compiled");

		return std::vector<char>();

		//return ReadFile(path);
	}

	MeshCreationData FileManager::LoadModelObj(std::string assetPath)
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

		return { vertices, indices };
	}
}

