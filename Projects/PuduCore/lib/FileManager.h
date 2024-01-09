#pragma once
#include <fstream>
#include <vector>
#include <format>

#include "Mesh.h"
#include "MeshCreationData.h"

static const std::string ASSETS_FOLDER_PATH = "Assets";
namespace Pudu {
    class FileManager
    {
    public:
        static std::vector<char> ReadFile(const std::string& fileName);

        /// <summary>
        /// Get path relative to assets folder
        /// </summary>
        static std::string GetAssetPath(std::string path);

        /// <summary>
        /// Read asset relative to the assets folder
        /// </summary>
        static std::vector<char> ReadAssetFile(const std::string& fileName);


        static std::vector<char> ReadShaderFile(const std::string& shaderPath);

        static MeshCreationData LoadModelObj(std::string assetPath);
    };

}
