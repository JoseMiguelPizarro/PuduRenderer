#pragma once
#include <concepts>

namespace Pudu
{
	class GPUResourceType {

		enum 
		{
			Texture,
			Buffer,
			RenderPass,
			UNINITIALIZED
		};
	};


	struct GPUResourceHandle
	{
		uint32_t index;
	};

	class GPUResource
	{
	public:
		GPUResourceHandle Handle() { return m_handle; }
		bool IsAllocated() { return m_allocated; }
		std::string name;



		void Create(PuduGraphics* gpu) {
			if (m_allocated)
			{
				return;
			}

			OnCreate(gpu);
			m_allocated = true;
		};

		virtual GPUResourceType Type() { return GPUResourceType::UNINITIALIZED; };

	private:
		friend class GPUResourcesManager;
		GPUResourceHandle m_handle;
		bool m_allocated;
		virtual void OnCreate(PuduGraphics* gpu);
	};


	struct TextureHandle :public GPUResourceHandle
	{
	}; // struct TextureHandle
}