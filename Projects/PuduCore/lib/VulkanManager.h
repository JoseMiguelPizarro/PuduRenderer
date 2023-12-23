#pragma once
#include <optional>
#include <cstdint>


typedef std::optional<uint32_t> Optional;

struct QueueFamilyIndices {
	Optional graphicsFamily;
	Optional presentFamily;

	bool isComplete() {
		return graphicsFamily.has_value() && presentFamily.has_value();
	}
};