#pragma once
#include <cstdint>
#include <unordered_map>
#include <string>
#include <vulkan/vulkan_core.h>

namespace Pudu
{
	static uint64_t hash(char const* s) {
		return std::hash<char const*>{}(s);
	}

	template <typename T>
	struct ResourcePool
	{
		T* GetResourcePtr(GPUResourceHandleBase handle);

		T GetResource(GPUResourceHandleBase handle);

		uint32_t AddResource(T resource);

		/// <summary>
		/// Allocates a new resource and returns its handle
		/// </summary>
		/// <typeparam name="T"></typeparam>
		/// <returns></returns>
		uint32_t ObtainResource();

		size_t Size();

		std::vector<T> GetAllResources();

	private:

		friend class GPUResourcesManager;

		std::vector<T> m_resources;
	};
}

#include "ResourcesPool.ipp"





