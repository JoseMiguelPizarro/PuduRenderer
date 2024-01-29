#pragma once
#include <fstream>
#include <vector>
#include <fastgltf/types.hpp>

#include "Mesh.h"
#include "MeshCreationData.h"
#include <filesystem>
#include <RenderEntity.h>
#include <fastgltf/parser.hpp>


namespace fs = std::filesystem;
namespace fg = fastgltf;

namespace Pudu {
	typedef fastgltf::Expected<fastgltf::Asset> GltfAsset;

	static const fs::path ASSETS_FOLDER_PATH = fs::path("../../../Assets");
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

		static std::vector<EntitySPtr> LoadGltfScene(fs::path const& path);

		static std::vector<char> LoadShader(fs::path const& path);

	private:
		static std::vector<MeshCreationData> GetGltfMeshCreationData(fs::path const& path, GltfAsset& gltfAsset);
		static GltfAsset LoadGltfAsset(fs::path const& path);
	};

}
