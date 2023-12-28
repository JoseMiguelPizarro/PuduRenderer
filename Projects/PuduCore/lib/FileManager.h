#pragma once

#include <fstream>
#include <vector>
#include <iostream>
#include <filesystem>
namespace fs = std::filesystem;

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