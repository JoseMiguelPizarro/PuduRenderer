#pragma once

#include <filesystem>

struct MaterialCreationData
{
	std::string name;
	bool hasBaseTexture = false;
	bool hasNormalMap = false;
	std::filesystem::path BaseTexturePath;
	std::filesystem::path NormalMapPath;
};

