#pragma once
#include <fstream>
#include <vector>

#include "Mesh.h"
#include "MeshCreationData.h"
#include <filesystem>


static const std::filesystem::path ASSETS_FOLDER_PATH = std::filesystem::path( "../../../Assets");

namespace Pudu {
	class FileManager
	{
	public:
		static std::vector<char> ReadFile(const std::filesystem::path fileName);

		/// <summary>
		/// Get path relative to assets folder
		/// </summary>
		static std::filesystem::path GetAssetPath(std::filesystem::path path);

		/// <summary>
		/// Read asset relative to the assets folder
		/// </summary>
		static std::vector<char> ReadAssetFile(const std::string& fileName);


		static std::vector<char> ReadShaderFile(const std::string& shaderPath);

		static MeshCreationData LoadModelObj(std::string assetPath);

		static std::vector<MeshCreationData> LoadModelGltf(std::filesystem::path path);
	};

}
