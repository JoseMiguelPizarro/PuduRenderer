#pragma once
#include <concepts>

namespace Pudu
{
	class PuduGraphics;

	class GPUResourceType {

	public:	enum Type
	{
		Texture,
		Buffer,
		RenderPass,
		UNINITIALIZED
	};
	};


	struct GPUResourceHandle
	{
	public:
		uint32_t index;


		bool IsEqual(const GPUResourceHandle& other) {
			return this->index == other.index;
		}
	};

	class GPUResource
	{
	public:
		GPUResourceHandle Handle() { return m_handle; }
		bool IsAllocated() { return m_allocated; }
		std::string name;


		virtual void Create(PuduGraphics* gpu) {
			if (m_allocated)
			{
				return;
			}

			OnCreate(gpu);
			m_allocated = true;
		};

		virtual GPUResourceType::Type Type() { return GPUResourceType::UNINITIALIZED; };

	protected:
		friend class GPUResourcesManager;
		GPUResourceHandle m_handle;
		bool m_allocated;
		virtual void OnCreate(PuduGraphics* gpu) {};
	};

	struct TextureHandle : public GPUResourceHandle
	{

	}; // struct TextureHandle
}