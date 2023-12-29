#pragma once

#include <fstream>
#include <vector>
#include <format>


static const std::string ASSETS_FOLDER_PATH = "Assets";


static std::vector<char> ReadFile(const std::string& fileName) {
	std::ifstream file(fileName, std::ios::ate | std::ios::binary); //ate: at the end lmao

	if (!file.is_open())
	{
		throw std::runtime_error(std::format("Failed to open file {}", fileName));
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
static std::string GetAssetPath(std::string path) {
	return std::format("{}/{}", ASSETS_FOLDER_PATH, path);
}

/// <summary>
/// Read asset relative to the assets folder
/// </summary>
static std::vector<char> ReadAssetFile(const std::string& fileName) {
	auto path = std::format("{}/{}", ASSETS_FOLDER_PATH, fileName);

	return ReadFile(path);
}
