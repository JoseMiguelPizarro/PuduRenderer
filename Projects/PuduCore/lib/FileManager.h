//NOTE: ONLY INCLUDE THIS IN A .CPP FILE. Reason: It needs to compile some templates

#pragma once
#define NOMINMAX
#include <complex.h>
#include <complex.h>
#include <filesystem>
#include <vector>

#include "BlitRenderPass.h"
#include "BlitRenderPass.h"
#include "MeshCreationData.h"
#include "RenderEntity.h"

namespace fs = std::filesystem;

namespace Pudu {
	static const fs::path ASSETS_FOLDER_PATH = fs::path("Assets");
	class FileManager
	{
	public:
		static std::vector<char> ReadFile(std::filesystem::path const& fileName);
		/// <summary>
		/// Get path relative to assets folder
		/// </summary>
		static std::filesystem::path GetAssetPath(fs::path const& path);

		/// <summary>
		/// Read asset relative to the assets folder
		/// </summary>
		static std::vector<char> ReadAssetFile(const std::string& fileName);
		static std::vector<char> ReadAssetFile(const fs::path& path);

		static MeshCreationData LoadModelObj(std::string const& assetPath);

		static std::vector<MeshCreationData> LoadModelGltf(fs::path const& path);

		///<summary> Load a gltf scene and returns its root node</summary>
		static EntitySPtr LoadGltfScene(fs::path const& path);

		static std::vector<char> LoadShader(fs::path const& path);

		static std::vector<glm::vec4> LoadPointCloud(fs::path const& path);
	};

}
