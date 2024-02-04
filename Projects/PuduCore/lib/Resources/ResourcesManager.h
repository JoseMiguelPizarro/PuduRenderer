#pragma once
#include <cstdint>
#include <unordered_map>
#include <string>
#include <typeinfo>
#include "Resources/Resources.h"
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
		T* GetResource(uint32_t handle);
		uint32_t AddResource(T resource);
		uint32_t ObtainResource();

	private:
		std::vector<T> m_resources;
	};


	class ResourcesManager {
	public:
		template <class T>
		T* GetResource(uint32_t handle);

		template<class T>
		uint32_t AddResource(T resource);


		static VkFormat VkFormatFromString(char const* format);

	private:
		std::unordered_map<type_info, BaseResourcePool> m_resourcesPools;


	};

}








