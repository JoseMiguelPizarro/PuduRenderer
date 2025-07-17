#pragma once

namespace Pudu
{
	template<typename T>
	T* ResourcePool<T>::GetResourcePtr(Handle handle)
	{
		if (handle < m_resources.size())
		{
			return &m_resources[handle];
		}

		return nullptr;
	}

	template<typename T>
	T ResourcePool<T>::GetResource(Handle handle)
	{
		return m_resources[handle];
	}

	template<typename T>
	std::vector<T> ResourcePool<T>::GetAllResources()
	{
		return m_resources;
	}


	template<typename T>
	Handle ResourcePool<T>::AddResource(T resource)
	{
		Handle id = (Handle)m_resources.size();
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
	Handle ResourcePool<T>::ObtainResource()
	{
		T t;
		m_resources.push_back(t);
		return (Handle)m_resources.size() - 1;
	}

	template<typename T>
	size_t ResourcePool<T>::Size()
	{
		return m_resources.size();
	}

}



