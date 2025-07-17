#pragma once
#include <cstdint>
#include <unordered_map>
#include <string>
#include <vulkan/vulkan_core.h>
#include "Resources/GPUResource.h"

namespace Pudu
{
	constexpr Size MAX_RESOURCE_SIZE = 1024;

	static uint64_t hash(char const* s) {
		return std::hash<char const*>{}(s);
	}

	template <typename T>
	struct ResourcePool
	{
		ResourcePool()
		{
			m_resources.reserve(MAX_RESOURCE_SIZE);
		}

		T* GetResourcePtr(Handle handle);

		T GetResource(Handle handle);

		Handle AddResource(T resource);

		/// <summary>
		/// Allocates a new resource and returns its handle
		/// </summary>
		/// <typeparam name="T"></typeparam>
		/// <returns></returns>
		Handle ObtainResource();

		Size Size();

		std::vector<T> GetAllResources();

	private:

		friend class GPUResourcesManager;

		std::vector<T> m_resources;
	};
}

#include "ResourcesPool.ipp"





