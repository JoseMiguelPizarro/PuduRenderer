#pragma once

namespace Pudu
{
	template<typename T>
	T* ResourcePool<T>::GetResourcePtr(uint32_t handle)
	{
		if (handle < m_resources.size())
		{
			return &m_resources[handle];
		}

		return nullptr;
	}

	template<typename T>
	T ResourcePool<T>::GetResource(uint32_t handle)
	{
		return m_resources[handle];
	}

	template<typename T>
	std::vector<T> ResourcePool<T>::GetAllResources()
	{
		return m_resources;
	}


	template<typename T>
	uint32_t ResourcePool<T>::AddResource(T resource)
	{
		uint32_t id = (uint32_t)m_resources.size();
		m_resources.resize(id + 1);
		m_resources[id] = resource;

		return id;
	}

	/// <summary>
	/// Allocates a new resource and returns its handle
	/// </summary>
	/// <typeparam name="T"></typeparam>
	/// <returns></returns>
	template<typename T>
	uint32_t ResourcePool<T>::ObtainResource()
	{
		T t;
		m_resources.push_back(t);
		return (uint32_t)m_resources.size() - 1;
	}

	template<typename T>
	size_t ResourcePool<T>::Size()
	{
		return m_resources.size();
	}

}



