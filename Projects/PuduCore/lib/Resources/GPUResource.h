#pragma once
#include <concepts>
#include <string>
#include "PuduConstants.h"
#include "PuduCore.h"

namespace Pudu
{
	class PuduGraphics;

	class GPUResourceType
	{
	public:
		enum Type
		{
			Texture,
			Buffer,
			RenderPass,
			UNINITIALIZED
		};
	};

	struct GPUResourceHandleBase
	{
		uint32_t Index() const { return m_Index; }

		friend class GPUResourcesManager;
		void SetIndex(uint32_t index) { m_Index = index; }
		uint32_t m_Index = k_INVALID_HANDLE;

		bool IsValid() { return m_Index != k_INVALID_HANDLE; }


		operator uint32_t& () { return m_Index; }

		bool operator==(const GPUResourceHandleBase &other) const
		{
			return m_Index == other.m_Index;
		};
	};

	struct GPUResourceHasher
	{
		size_t operator()(const GPUResourceHandleBase& k) const
		{
			return std::hash<uint32_t>()(k.m_Index);
		}
	};

	class GPUResourceBase
	{
	public:
		bool IsAllocated() const { return m_allocated; }
		std::string name;

		void Create(PuduGraphics* gpu)
		{
			m_gpu = gpu;

			if (m_allocated)
			{
				return;
			}

			OnCreate(gpu);
			m_allocated = true;
		}

	protected:
		friend class GPUResourcesManager;
		bool m_allocated = false;
		PuduGraphics* m_gpu;

		virtual void OnCreate(PuduGraphics* gpu)
		{
		};
	};



	template <typename T>
	//requires(std::convertible_to<T, GPUResourceBase>)
	struct GPUResourceHandle : public GPUResourceHandleBase
	{
	public:
		bool IsEqual(const GPUResourceHandle& other)
		{
			return this->m_Index == other.m_Index;
		}
	};


	template <typename T>
	class GPUResource : public GPUResourceBase
	{
	public:
		GPUResourceHandle<T> Handle() { return m_handle; }


		virtual GPUResourceType::Type Type() { return GPUResourceType::UNINITIALIZED; };
		virtual ~GPUResource() = default;

	protected:
		friend class GPUResourcesManager;
		GPUResourceHandle<T> m_handle;
	};
}
