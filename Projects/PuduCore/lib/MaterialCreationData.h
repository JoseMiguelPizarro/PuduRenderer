#pragma once

#include <filesystem>

struct MaterialCreationData
{
	std::string name;
	bool hasBaseTexture = false;
	bool hasNormalMap = false;
	bool hasMetallicRoughness = false;
	bool hasEmissiveTexture = false;
	std::filesystem::path BaseTexturePath;
	std::filesystem::path NormalMapPath;
	std::filesystem::path MetallicRoughnessPath;
	std::filesystem::path EmissivePath;
};

