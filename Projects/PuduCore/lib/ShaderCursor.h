#include <vulkan/vulkan_core.h>
#include <cstddef>
#include <slang-gfx.h>
#include <glm/vec3.hpp>
#include <GraphicsBuffer.h>
#include "ShaderObject.h"

using namespace glm;

namespace Pudu
{
	class PuduGraphics;
	class Texture;

	class ShaderCursor {
	public:
		/// <summary>
		/// Form a cursor to a field of a struct
		/// </summary>
		ShaderCursor field(const char* name);

		/// <summary>
		/// Forms a cursor to a field of a struct
		/// </summary>s
		ShaderCursor field(int index);
		/// <summary>
		/// Forms a cursor to an element of an array
		/// </summary>
		ShaderCursor element(int index);


		void Write(Texture* texture);
		void Write(TextureSampler* sampler);
		void Write(const void* data, size_t size);
		void Write(vec3 value);

	private:
		std::byte* m_bufferdata;
		VkDescriptorSet m_descriptorset;
		slang::TypeLayoutReflection* m_typeLayout;
		PuduGraphics* m_graphics;
		ShaderObject* m_object = nullptr;
		ShaderOffset m_offset;
	};
}