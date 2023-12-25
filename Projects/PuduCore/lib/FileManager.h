#pragma once

#include <fstream>
#include <vector>
#include <iostream>
#include <filesystem>
namespace fs = std::filesystem;

static std::vector<char> ReadFile(const std::string& fileName) {
	if (!file.is_open())
	{
		throw std::runtime_error("Failed to open file");
	}

	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);

	file.seekg(0); //Go back to the beggining of the file
	file.read(buffer.data(), fileSize);

	file.close();

	return buffer;
}