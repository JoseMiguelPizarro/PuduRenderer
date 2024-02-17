#pragma once
#include <cstdint>
#include <unordered_map>
#include <string>
#include <typeinfo>
#include <vulkan/vulkan_core.h>

namespace Pudu
{
	static uint64_t hash(char const* s) {
		return std::hash<char const *>{}(s);
	}

	struct BaseResourcePool
	{

	};

	template <typename T>
	struct ResourcePool : BaseResourcePool
	{
		T* GetResourcePtr(uint32_t handle);
		T GetResource(uint32_t handle);

		uint32_t AddResource(T resource);
		uint32_t ObtainResource();

		size_t Size();

	private:
		std::vector<T> m_resources;
	};
}








