#include <filesystem>
#include <simdjson.h>
#include "Logger.h"
#include "FrameGraph/FrameGraph.h"
#include "FrameGraph/RenderPass.h"
#include <Resources/FrameBufferCreationData.h>
#include "Texture2D.h"
#include <algorithm>

namespace Pudu
{

#pragma region Utils

	TextureType::Enum TextureTypeFromString(char const* texType) {
		if (strcmp(texType, "cube"))
		{
			return TextureType::Texture_Cube;
		}

		return TextureType::Texture2D;
	}

	VkFormat  VkFormatFromString(char const* format) {
		if (strcmp(format, "VK_FORMAT_R4G4_UNORM_PACK8") == 0) {
			return VK_FORMAT_R4G4_UNORM_PACK8;
		}
		if (strcmp(format, "VK_FORMAT_R4G4B4A4_UNORM_PACK16") == 0) {
			return VK_FORMAT_R4G4B4A4_UNORM_PACK16;
		}
		if (strcmp(format, "VK_FORMAT_B4G4R4A4_UNORM_PACK16") == 0) {
			return VK_FORMAT_B4G4R4A4_UNORM_PACK16;
		}
		if (strcmp(format, "VK_FORMAT_R5G6B5_UNORM_PACK16") == 0) {
			return VK_FORMAT_R5G6B5_UNORM_PACK16;
		}
		if (strcmp(format, "VK_FORMAT_B5G6R5_UNORM_PACK16") == 0) {
			return VK_FORMAT_B5G6R5_UNORM_PACK16;
		}
		if (strcmp(format, "VK_FORMAT_R5G5B5A1_UNORM_PACK16") == 0) {
			return VK_FORMAT_R5G5B5A1_UNORM_PACK16;
		}
		if (strcmp(format, "VK_FORMAT_B5G5R5A1_UNORM_PACK16") == 0) {
			return VK_FORMAT_B5G5R5A1_UNORM_PACK16;
		}
		if (strcmp(format, "VK_FORMAT_A1R5G5B5_UNORM_PACK16") == 0) {
			return VK_FORMAT_A1R5G5B5_UNORM_PACK16;
		}
		if (strcmp(format, "VK_FORMAT_R8_UNORM") == 0) {
			return VK_FORMAT_R8_UNORM;
		}
		if (strcmp(format, "VK_FORMAT_R8_SNORM") == 0) {
			return VK_FORMAT_R8_SNORM;
		}
		if (strcmp(format, "VK_FORMAT_R8_USCALED") == 0) {
			return VK_FORMAT_R8_USCALED;
		}
		if (strcmp(format, "VK_FORMAT_R8_SSCALED") == 0) {
			return VK_FORMAT_R8_SSCALED;
		}
		if (strcmp(format, "VK_FORMAT_R8_UINT") == 0) {
			return VK_FORMAT_R8_UINT;
		}
		if (strcmp(format, "VK_FORMAT_R8_SINT") == 0) {
			return VK_FORMAT_R8_SINT;
		}
		if (strcmp(format, "VK_FORMAT_R8_SRGB") == 0) {
			return VK_FORMAT_R8_SRGB;
		}
		if (strcmp(format, "VK_FORMAT_R8G8_UNORM") == 0) {
			return VK_FORMAT_R8G8_UNORM;
		}
		if (strcmp(format, "VK_FORMAT_R8G8_SNORM") == 0) {
			return VK_FORMAT_R8G8_SNORM;
		}
		if (strcmp(format, "VK_FORMAT_R8G8_USCALED") == 0) {
			return VK_FORMAT_R8G8_USCALED;
		}
		if (strcmp(format, "VK_FORMAT_R8G8_SSCALED") == 0) {
			return VK_FORMAT_R8G8_SSCALED;
		}
		if (strcmp(format, "VK_FORMAT_R8G8_UINT") == 0) {
			return VK_FORMAT_R8G8_UINT;
		}
		if (strcmp(format, "VK_FORMAT_R8G8_SINT") == 0) {
			return VK_FORMAT_R8G8_SINT;
		}
		if (strcmp(format, "VK_FORMAT_R8G8_SRGB") == 0) {
			return VK_FORMAT_R8G8_SRGB;
		}
		if (strcmp(format, "VK_FORMAT_R8G8B8_UNORM") == 0) {
			return VK_FORMAT_R8G8B8_UNORM;
		}
		if (strcmp(format, "VK_FORMAT_R8G8B8_SNORM") == 0) {
			return VK_FORMAT_R8G8B8_SNORM;
		}
		if (strcmp(format, "VK_FORMAT_R8G8B8_USCALED") == 0) {
			return VK_FORMAT_R8G8B8_USCALED;
		}
		if (strcmp(format, "VK_FORMAT_R8G8B8_SSCALED") == 0) {
			return VK_FORMAT_R8G8B8_SSCALED;
		}
		if (strcmp(format, "VK_FORMAT_R8G8B8_UINT") == 0) {
			return VK_FORMAT_R8G8B8_UINT;
		}
		if (strcmp(format, "VK_FORMAT_R8G8B8_SINT") == 0) {
			return VK_FORMAT_R8G8B8_SINT;
		}
		if (strcmp(format, "VK_FORMAT_R8G8B8_SRGB") == 0) {
			return VK_FORMAT_R8G8B8_SRGB;
		}
		if (strcmp(format, "VK_FORMAT_B8G8R8_UNORM") == 0) {
			return VK_FORMAT_B8G8R8_UNORM;
		}
		if (strcmp(format, "VK_FORMAT_B8G8R8_SNORM") == 0) {
			return VK_FORMAT_B8G8R8_SNORM;
		}
		if (strcmp(format, "VK_FORMAT_B8G8R8_USCALED") == 0) {
			return VK_FORMAT_B8G8R8_USCALED;
		}
		if (strcmp(format, "VK_FORMAT_B8G8R8_SSCALED") == 0) {
			return VK_FORMAT_B8G8R8_SSCALED;
		}
		if (strcmp(format, "VK_FORMAT_B8G8R8_UINT") == 0) {
			return VK_FORMAT_B8G8R8_UINT;
		}
		if (strcmp(format, "VK_FORMAT_B8G8R8_SINT") == 0) {
			return VK_FORMAT_B8G8R8_SINT;
		}
		if (strcmp(format, "VK_FORMAT_B8G8R8_SRGB") == 0) {
			return VK_FORMAT_B8G8R8_SRGB;
		}
		if (strcmp(format, "VK_FORMAT_R8G8B8A8_UNORM") == 0) {
			return VK_FORMAT_R8G8B8A8_UNORM;
		}
		if (strcmp(format, "VK_FORMAT_R8G8B8A8_SNORM") == 0) {
			return VK_FORMAT_R8G8B8A8_SNORM;
		}
		if (strcmp(format, "VK_FORMAT_R8G8B8A8_USCALED") == 0) {
			return VK_FORMAT_R8G8B8A8_USCALED;
		}
		if (strcmp(format, "VK_FORMAT_R8G8B8A8_SSCALED") == 0) {
			return VK_FORMAT_R8G8B8A8_SSCALED;
		}
		if (strcmp(format, "VK_FORMAT_R8G8B8A8_UINT") == 0) {
			return VK_FORMAT_R8G8B8A8_UINT;
		}
		if (strcmp(format, "VK_FORMAT_R8G8B8A8_SINT") == 0) {
			return VK_FORMAT_R8G8B8A8_SINT;
		}
		if (strcmp(format, "VK_FORMAT_R8G8B8A8_SRGB") == 0) {
			return VK_FORMAT_R8G8B8A8_SRGB;
		}
		if (strcmp(format, "VK_FORMAT_B8G8R8A8_UNORM") == 0) {
			return VK_FORMAT_B8G8R8A8_UNORM;
		}
		if (strcmp(format, "VK_FORMAT_B8G8R8A8_SNORM") == 0) {
			return VK_FORMAT_B8G8R8A8_SNORM;
		}
		if (strcmp(format, "VK_FORMAT_B8G8R8A8_USCALED") == 0) {
			return VK_FORMAT_B8G8R8A8_USCALED;
		}
		if (strcmp(format, "VK_FORMAT_B8G8R8A8_SSCALED") == 0) {
			return VK_FORMAT_B8G8R8A8_SSCALED;
		}
		if (strcmp(format, "VK_FORMAT_B8G8R8A8_UINT") == 0) {
			return VK_FORMAT_B8G8R8A8_UINT;
		}
		if (strcmp(format, "VK_FORMAT_B8G8R8A8_SINT") == 0) {
			return VK_FORMAT_B8G8R8A8_SINT;
		}
		if (strcmp(format, "VK_FORMAT_B8G8R8A8_SRGB") == 0) {
			return VK_FORMAT_B8G8R8A8_SRGB;
		}
		if (strcmp(format, "VK_FORMAT_A8B8G8R8_UNORM_PACK32") == 0) {
			return VK_FORMAT_A8B8G8R8_UNORM_PACK32;
		}
		if (strcmp(format, "VK_FORMAT_A8B8G8R8_SNORM_PACK32") == 0) {
			return VK_FORMAT_A8B8G8R8_SNORM_PACK32;
		}
		if (strcmp(format, "VK_FORMAT_A8B8G8R8_USCALED_PACK32") == 0) {
			return VK_FORMAT_A8B8G8R8_USCALED_PACK32;
		}
		if (strcmp(format, "VK_FORMAT_A8B8G8R8_SSCALED_PACK32") == 0) {
			return VK_FORMAT_A8B8G8R8_SSCALED_PACK32;
		}
		if (strcmp(format, "VK_FORMAT_A8B8G8R8_UINT_PACK32") == 0) {
			return VK_FORMAT_A8B8G8R8_UINT_PACK32;
		}
		if (strcmp(format, "VK_FORMAT_A8B8G8R8_SINT_PACK32") == 0) {
			return VK_FORMAT_A8B8G8R8_SINT_PACK32;
		}
		if (strcmp(format, "VK_FORMAT_A8B8G8R8_SRGB_PACK32") == 0) {
			return VK_FORMAT_A8B8G8R8_SRGB_PACK32;
		}
		if (strcmp(format, "VK_FORMAT_A2R10G10B10_UNORM_PACK32") == 0) {
			return VK_FORMAT_A2R10G10B10_UNORM_PACK32;
		}
		if (strcmp(format, "VK_FORMAT_A2R10G10B10_SNORM_PACK32") == 0) {
			return VK_FORMAT_A2R10G10B10_SNORM_PACK32;
		}
		if (strcmp(format, "VK_FORMAT_A2R10G10B10_USCALED_PACK32") == 0) {
			return VK_FORMAT_A2R10G10B10_USCALED_PACK32;
		}
		if (strcmp(format, "VK_FORMAT_A2R10G10B10_SSCALED_PACK32") == 0) {
			return VK_FORMAT_A2R10G10B10_SSCALED_PACK32;
		}
		if (strcmp(format, "VK_FORMAT_A2R10G10B10_UINT_PACK32") == 0) {
			return VK_FORMAT_A2R10G10B10_UINT_PACK32;
		}
		if (strcmp(format, "VK_FORMAT_A2R10G10B10_SINT_PACK32") == 0) {
			return VK_FORMAT_A2R10G10B10_SINT_PACK32;
		}
		if (strcmp(format, "VK_FORMAT_A2B10G10R10_UNORM_PACK32") == 0) {
			return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
		}
		if (strcmp(format, "VK_FORMAT_A2B10G10R10_SNORM_PACK32") == 0) {
			return VK_FORMAT_A2B10G10R10_SNORM_PACK32;
		}
		if (strcmp(format, "VK_FORMAT_A2B10G10R10_USCALED_PACK32") == 0) {
			return VK_FORMAT_A2B10G10R10_USCALED_PACK32;
		}
		if (strcmp(format, "VK_FORMAT_A2B10G10R10_SSCALED_PACK32") == 0) {
			return VK_FORMAT_A2B10G10R10_SSCALED_PACK32;
		}
		if (strcmp(format, "VK_FORMAT_A2B10G10R10_UINT_PACK32") == 0) {
			return VK_FORMAT_A2B10G10R10_UINT_PACK32;
		}
		if (strcmp(format, "VK_FORMAT_A2B10G10R10_SINT_PACK32") == 0) {
			return VK_FORMAT_A2B10G10R10_SINT_PACK32;
		}
		if (strcmp(format, "VK_FORMAT_R16_UNORM") == 0) {
			return VK_FORMAT_R16_UNORM;
		}
		if (strcmp(format, "VK_FORMAT_R16_SNORM") == 0) {
			return VK_FORMAT_R16_SNORM;
		}
		if (strcmp(format, "VK_FORMAT_R16_USCALED") == 0) {
			return VK_FORMAT_R16_USCALED;
		}
		if (strcmp(format, "VK_FORMAT_R16_SSCALED") == 0) {
			return VK_FORMAT_R16_SSCALED;
		}
		if (strcmp(format, "VK_FORMAT_R16_UINT") == 0) {
			return VK_FORMAT_R16_UINT;
		}
		if (strcmp(format, "VK_FORMAT_R16_SINT") == 0) {
			return VK_FORMAT_R16_SINT;
		}
		if (strcmp(format, "VK_FORMAT_R16_SFLOAT") == 0) {
			return VK_FORMAT_R16_SFLOAT;
		}
		if (strcmp(format, "VK_FORMAT_R16G16_UNORM") == 0) {
			return VK_FORMAT_R16G16_UNORM;
		}
		if (strcmp(format, "VK_FORMAT_R16G16_SNORM") == 0) {
			return VK_FORMAT_R16G16_SNORM;
		}
		if (strcmp(format, "VK_FORMAT_R16G16_USCALED") == 0) {
			return VK_FORMAT_R16G16_USCALED;
		}
		if (strcmp(format, "VK_FORMAT_R16G16_SSCALED") == 0) {
			return VK_FORMAT_R16G16_SSCALED;
		}
		if (strcmp(format, "VK_FORMAT_R16G16_UINT") == 0) {
			return VK_FORMAT_R16G16_UINT;
		}
		if (strcmp(format, "VK_FORMAT_R16G16_SINT") == 0) {
			return VK_FORMAT_R16G16_SINT;
		}
		if (strcmp(format, "VK_FORMAT_R16G16_SFLOAT") == 0) {
			return VK_FORMAT_R16G16_SFLOAT;
		}
		if (strcmp(format, "VK_FORMAT_R16G16B16_UNORM") == 0) {
			return VK_FORMAT_R16G16B16_UNORM;
		}
		if (strcmp(format, "VK_FORMAT_R16G16B16_SNORM") == 0) {
			return VK_FORMAT_R16G16B16_SNORM;
		}
		if (strcmp(format, "VK_FORMAT_R16G16B16_USCALED") == 0) {
			return VK_FORMAT_R16G16B16_USCALED;
		}
		if (strcmp(format, "VK_FORMAT_R16G16B16_SSCALED") == 0) {
			return VK_FORMAT_R16G16B16_SSCALED;
		}
		if (strcmp(format, "VK_FORMAT_R16G16B16_UINT") == 0) {
			return VK_FORMAT_R16G16B16_UINT;
		}
		if (strcmp(format, "VK_FORMAT_R16G16B16_SINT") == 0) {
			return VK_FORMAT_R16G16B16_SINT;
		}
		if (strcmp(format, "VK_FORMAT_R16G16B16_SFLOAT") == 0) {
			return VK_FORMAT_R16G16B16_SFLOAT;
		}
		if (strcmp(format, "VK_FORMAT_R16G16B16A16_UNORM") == 0) {
			return VK_FORMAT_R16G16B16A16_UNORM;
		}
		if (strcmp(format, "VK_FORMAT_R16G16B16A16_SNORM") == 0) {
			return VK_FORMAT_R16G16B16A16_SNORM;
		}
		if (strcmp(format, "VK_FORMAT_R16G16B16A16_USCALED") == 0) {
			return VK_FORMAT_R16G16B16A16_USCALED;
		}
		if (strcmp(format, "VK_FORMAT_R16G16B16A16_SSCALED") == 0) {
			return VK_FORMAT_R16G16B16A16_SSCALED;
		}
		if (strcmp(format, "VK_FORMAT_R16G16B16A16_UINT") == 0) {
			return VK_FORMAT_R16G16B16A16_UINT;
		}
		if (strcmp(format, "VK_FORMAT_R16G16B16A16_SINT") == 0) {
			return VK_FORMAT_R16G16B16A16_SINT;
		}
		if (strcmp(format, "VK_FORMAT_R16G16B16A16_SFLOAT") == 0) {
			return VK_FORMAT_R16G16B16A16_SFLOAT;
		}
		if (strcmp(format, "VK_FORMAT_R32_UINT") == 0) {
			return VK_FORMAT_R32_UINT;
		}
		if (strcmp(format, "VK_FORMAT_R32_SINT") == 0) {
			return VK_FORMAT_R32_SINT;
		}
		if (strcmp(format, "VK_FORMAT_R32_SFLOAT") == 0) {
			return VK_FORMAT_R32_SFLOAT;
		}
		if (strcmp(format, "VK_FORMAT_R32G32_UINT") == 0) {
			return VK_FORMAT_R32G32_UINT;
		}
		if (strcmp(format, "VK_FORMAT_R32G32_SINT") == 0) {
			return VK_FORMAT_R32G32_SINT;
		}
		if (strcmp(format, "VK_FORMAT_R32G32_SFLOAT") == 0) {
			return VK_FORMAT_R32G32_SFLOAT;
		}
		if (strcmp(format, "VK_FORMAT_R32G32B32_UINT") == 0) {
			return VK_FORMAT_R32G32B32_UINT;
		}
		if (strcmp(format, "VK_FORMAT_R32G32B32_SINT") == 0) {
			return VK_FORMAT_R32G32B32_SINT;
		}
		if (strcmp(format, "VK_FORMAT_R32G32B32_SFLOAT") == 0) {
			return VK_FORMAT_R32G32B32_SFLOAT;
		}
		if (strcmp(format, "VK_FORMAT_R32G32B32A32_UINT") == 0) {
			return VK_FORMAT_R32G32B32A32_UINT;
		}
		if (strcmp(format, "VK_FORMAT_R32G32B32A32_SINT") == 0) {
			return VK_FORMAT_R32G32B32A32_SINT;
		}
		if (strcmp(format, "VK_FORMAT_R32G32B32A32_SFLOAT") == 0) {
			return VK_FORMAT_R32G32B32A32_SFLOAT;
		}
		if (strcmp(format, "VK_FORMAT_R64_UINT") == 0) {
			return VK_FORMAT_R64_UINT;
		}
		if (strcmp(format, "VK_FORMAT_R64_SINT") == 0) {
			return VK_FORMAT_R64_SINT;
		}
		if (strcmp(format, "VK_FORMAT_R64_SFLOAT") == 0) {
			return VK_FORMAT_R64_SFLOAT;
		}
		if (strcmp(format, "VK_FORMAT_R64G64_UINT") == 0) {
			return VK_FORMAT_R64G64_UINT;
		}
		if (strcmp(format, "VK_FORMAT_R64G64_SINT") == 0) {
			return VK_FORMAT_R64G64_SINT;
		}
		if (strcmp(format, "VK_FORMAT_R64G64_SFLOAT") == 0) {
			return VK_FORMAT_R64G64_SFLOAT;
		}
		if (strcmp(format, "VK_FORMAT_R64G64B64_UINT") == 0) {
			return VK_FORMAT_R64G64B64_UINT;
		}
		if (strcmp(format, "VK_FORMAT_R64G64B64_SINT") == 0) {
			return VK_FORMAT_R64G64B64_SINT;
		}
		if (strcmp(format, "VK_FORMAT_R64G64B64_SFLOAT") == 0) {
			return VK_FORMAT_R64G64B64_SFLOAT;
		}
		if (strcmp(format, "VK_FORMAT_R64G64B64A64_UINT") == 0) {
			return VK_FORMAT_R64G64B64A64_UINT;
		}
		if (strcmp(format, "VK_FORMAT_R64G64B64A64_SINT") == 0) {
			return VK_FORMAT_R64G64B64A64_SINT;
		}
		if (strcmp(format, "VK_FORMAT_R64G64B64A64_SFLOAT") == 0) {
			return VK_FORMAT_R64G64B64A64_SFLOAT;
		}
		if (strcmp(format, "VK_FORMAT_B10G11R11_UFLOAT_PACK32") == 0) {
			return VK_FORMAT_B10G11R11_UFLOAT_PACK32;
		}
		if (strcmp(format, "VK_FORMAT_E5B9G9R9_UFLOAT_PACK32") == 0) {
			return VK_FORMAT_E5B9G9R9_UFLOAT_PACK32;
		}
		if (strcmp(format, "VK_FORMAT_D16_UNORM") == 0) {
			return VK_FORMAT_D16_UNORM;
		}
		if (strcmp(format, "VK_FORMAT_X8_D24_UNORM_PACK32") == 0) {
			return VK_FORMAT_X8_D24_UNORM_PACK32;
		}
		if (strcmp(format, "VK_FORMAT_D32_SFLOAT") == 0) {
			return VK_FORMAT_D32_SFLOAT;
		}
		if (strcmp(format, "VK_FORMAT_S8_UINT") == 0) {
			return VK_FORMAT_S8_UINT;
		}
		if (strcmp(format, "VK_FORMAT_D16_UNORM_S8_UINT") == 0) {
			return VK_FORMAT_D16_UNORM_S8_UINT;
		}
		if (strcmp(format, "VK_FORMAT_D24_UNORM_S8_UINT") == 0) {
			return VK_FORMAT_D24_UNORM_S8_UINT;
		}
		if (strcmp(format, "VK_FORMAT_D32_SFLOAT_S8_UINT") == 0) {
			return VK_FORMAT_D32_SFLOAT_S8_UINT;
		}
		if (strcmp(format, "VK_FORMAT_BC1_RGB_UNORM_BLOCK") == 0) {
			return VK_FORMAT_BC1_RGB_UNORM_BLOCK;
		}
		if (strcmp(format, "VK_FORMAT_BC1_RGB_SRGB_BLOCK") == 0) {
			return VK_FORMAT_BC1_RGB_SRGB_BLOCK;
		}
		if (strcmp(format, "VK_FORMAT_BC1_RGBA_UNORM_BLOCK") == 0) {
			return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
		}
		if (strcmp(format, "VK_FORMAT_BC1_RGBA_SRGB_BLOCK") == 0) {
			return VK_FORMAT_BC1_RGBA_SRGB_BLOCK;
		}
		if (strcmp(format, "VK_FORMAT_BC2_UNORM_BLOCK") == 0) {
			return VK_FORMAT_BC2_UNORM_BLOCK;
		}
		if (strcmp(format, "VK_FORMAT_BC2_SRGB_BLOCK") == 0) {
			return VK_FORMAT_BC2_SRGB_BLOCK;
		}
		if (strcmp(format, "VK_FORMAT_BC3_UNORM_BLOCK") == 0) {
			return VK_FORMAT_BC3_UNORM_BLOCK;
		}
		if (strcmp(format, "VK_FORMAT_BC3_SRGB_BLOCK") == 0) {
			return VK_FORMAT_BC3_SRGB_BLOCK;
		}
		if (strcmp(format, "VK_FORMAT_BC4_UNORM_BLOCK") == 0) {
			return VK_FORMAT_BC4_UNORM_BLOCK;
		}
		if (strcmp(format, "VK_FORMAT_BC4_SNORM_BLOCK") == 0) {
			return VK_FORMAT_BC4_SNORM_BLOCK;
		}
		if (strcmp(format, "VK_FORMAT_BC5_UNORM_BLOCK") == 0) {
			return VK_FORMAT_BC5_UNORM_BLOCK;
		}
		if (strcmp(format, "VK_FORMAT_BC5_SNORM_BLOCK") == 0) {
			return VK_FORMAT_BC5_SNORM_BLOCK;
		}
		if (strcmp(format, "VK_FORMAT_BC6H_UFLOAT_BLOCK") == 0) {
			return VK_FORMAT_BC6H_UFLOAT_BLOCK;
		}
		if (strcmp(format, "VK_FORMAT_BC6H_SFLOAT_BLOCK") == 0) {
			return VK_FORMAT_BC6H_SFLOAT_BLOCK;
		}
		if (strcmp(format, "VK_FORMAT_BC7_UNORM_BLOCK") == 0) {
			return VK_FORMAT_BC7_UNORM_BLOCK;
		}
		if (strcmp(format, "VK_FORMAT_BC7_SRGB_BLOCK") == 0) {
			return VK_FORMAT_BC7_SRGB_BLOCK;
		}
		if (strcmp(format, "VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK") == 0) {
			return VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK;
		}
		if (strcmp(format, "VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK") == 0) {
			return VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK;
		}
		if (strcmp(format, "VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK") == 0) {
			return VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK;
		}
		if (strcmp(format, "VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK") == 0) {
			return VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK;
		}
		if (strcmp(format, "VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK") == 0) {
			return VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK;
		}
		if (strcmp(format, "VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK") == 0) {
			return VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK;
		}
		if (strcmp(format, "VK_FORMAT_EAC_R11_UNORM_BLOCK") == 0) {
			return VK_FORMAT_EAC_R11_UNORM_BLOCK;
		}
		if (strcmp(format, "VK_FORMAT_EAC_R11_SNORM_BLOCK") == 0) {
			return VK_FORMAT_EAC_R11_SNORM_BLOCK;
		}
		if (strcmp(format, "VK_FORMAT_EAC_R11G11_UNORM_BLOCK") == 0) {
			return VK_FORMAT_EAC_R11G11_UNORM_BLOCK;
		}
		if (strcmp(format, "VK_FORMAT_EAC_R11G11_SNORM_BLOCK") == 0) {
			return VK_FORMAT_EAC_R11G11_SNORM_BLOCK;
		}
		if (strcmp(format, "VK_FORMAT_ASTC_4x4_UNORM_BLOCK") == 0) {
			return VK_FORMAT_ASTC_4x4_UNORM_BLOCK;
		}
		if (strcmp(format, "VK_FORMAT_ASTC_4x4_SRGB_BLOCK") == 0) {
			return VK_FORMAT_ASTC_4x4_SRGB_BLOCK;
		}
		if (strcmp(format, "VK_FORMAT_ASTC_5x4_UNORM_BLOCK") == 0) {
			return VK_FORMAT_ASTC_5x4_UNORM_BLOCK;
		}
		if (strcmp(format, "VK_FORMAT_ASTC_5x4_SRGB_BLOCK") == 0) {
			return VK_FORMAT_ASTC_5x4_SRGB_BLOCK;
		}
		if (strcmp(format, "VK_FORMAT_ASTC_5x5_UNORM_BLOCK") == 0) {
			return VK_FORMAT_ASTC_5x5_UNORM_BLOCK;
		}
		if (strcmp(format, "VK_FORMAT_ASTC_5x5_SRGB_BLOCK") == 0) {
			return VK_FORMAT_ASTC_5x5_SRGB_BLOCK;
		}
		if (strcmp(format, "VK_FORMAT_ASTC_6x5_UNORM_BLOCK") == 0) {
			return VK_FORMAT_ASTC_6x5_UNORM_BLOCK;
		}
		if (strcmp(format, "VK_FORMAT_ASTC_6x5_SRGB_BLOCK") == 0) {
			return VK_FORMAT_ASTC_6x5_SRGB_BLOCK;
		}
		if (strcmp(format, "VK_FORMAT_ASTC_6x6_UNORM_BLOCK") == 0) {
			return VK_FORMAT_ASTC_6x6_UNORM_BLOCK;
		}
		if (strcmp(format, "VK_FORMAT_ASTC_6x6_SRGB_BLOCK") == 0) {
			return VK_FORMAT_ASTC_6x6_SRGB_BLOCK;
		}
		if (strcmp(format, "VK_FORMAT_ASTC_8x5_UNORM_BLOCK") == 0) {
			return VK_FORMAT_ASTC_8x5_UNORM_BLOCK;
		}
		if (strcmp(format, "VK_FORMAT_ASTC_8x5_SRGB_BLOCK") == 0) {
			return VK_FORMAT_ASTC_8x5_SRGB_BLOCK;
		}
		if (strcmp(format, "VK_FORMAT_ASTC_8x6_UNORM_BLOCK") == 0) {
			return VK_FORMAT_ASTC_8x6_UNORM_BLOCK;
		}
		if (strcmp(format, "VK_FORMAT_ASTC_8x6_SRGB_BLOCK") == 0) {
			return VK_FORMAT_ASTC_8x6_SRGB_BLOCK;
		}
		if (strcmp(format, "VK_FORMAT_ASTC_8x8_UNORM_BLOCK") == 0) {
			return VK_FORMAT_ASTC_8x8_UNORM_BLOCK;
		}
		if (strcmp(format, "VK_FORMAT_ASTC_8x8_SRGB_BLOCK") == 0) {
			return VK_FORMAT_ASTC_8x8_SRGB_BLOCK;
		}
		if (strcmp(format, "VK_FORMAT_ASTC_10x5_UNORM_BLOCK") == 0) {
			return VK_FORMAT_ASTC_10x5_UNORM_BLOCK;
		}
		if (strcmp(format, "VK_FORMAT_ASTC_10x5_SRGB_BLOCK") == 0) {
			return VK_FORMAT_ASTC_10x5_SRGB_BLOCK;
		}
		if (strcmp(format, "VK_FORMAT_ASTC_10x6_UNORM_BLOCK") == 0) {
			return VK_FORMAT_ASTC_10x6_UNORM_BLOCK;
		}
		if (strcmp(format, "VK_FORMAT_ASTC_10x6_SRGB_BLOCK") == 0) {
			return VK_FORMAT_ASTC_10x6_SRGB_BLOCK;
		}
		if (strcmp(format, "VK_FORMAT_ASTC_10x8_UNORM_BLOCK") == 0) {
			return VK_FORMAT_ASTC_10x8_UNORM_BLOCK;
		}
		if (strcmp(format, "VK_FORMAT_ASTC_10x8_SRGB_BLOCK") == 0) {
			return VK_FORMAT_ASTC_10x8_SRGB_BLOCK;
		}
		if (strcmp(format, "VK_FORMAT_ASTC_10x10_UNORM_BLOCK") == 0) {
			return VK_FORMAT_ASTC_10x10_UNORM_BLOCK;
		}
		if (strcmp(format, "VK_FORMAT_ASTC_10x10_SRGB_BLOCK") == 0) {
			return VK_FORMAT_ASTC_10x10_SRGB_BLOCK;
		}
		if (strcmp(format, "VK_FORMAT_ASTC_12x10_UNORM_BLOCK") == 0) {
			return VK_FORMAT_ASTC_12x10_UNORM_BLOCK;
		}
		if (strcmp(format, "VK_FORMAT_ASTC_12x10_SRGB_BLOCK") == 0) {
			return VK_FORMAT_ASTC_12x10_SRGB_BLOCK;
		}
		if (strcmp(format, "VK_FORMAT_ASTC_12x12_UNORM_BLOCK") == 0) {
			return VK_FORMAT_ASTC_12x12_UNORM_BLOCK;
		}
		if (strcmp(format, "VK_FORMAT_ASTC_12x12_SRGB_BLOCK") == 0) {
			return VK_FORMAT_ASTC_12x12_SRGB_BLOCK;
		}
		if (strcmp(format, "VK_FORMAT_G8B8G8R8_422_UNORM") == 0) {
			return VK_FORMAT_G8B8G8R8_422_UNORM;
		}
		if (strcmp(format, "VK_FORMAT_B8G8R8G8_422_UNORM") == 0) {
			return VK_FORMAT_B8G8R8G8_422_UNORM;
		}
		if (strcmp(format, "VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM") == 0) {
			return VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM;
		}
		if (strcmp(format, "VK_FORMAT_G8_B8R8_2PLANE_420_UNORM") == 0) {
			return VK_FORMAT_G8_B8R8_2PLANE_420_UNORM;
		}
		if (strcmp(format, "VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM") == 0) {
			return VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM;
		}
		if (strcmp(format, "VK_FORMAT_G8_B8R8_2PLANE_422_UNORM") == 0) {
			return VK_FORMAT_G8_B8R8_2PLANE_422_UNORM;
		}
		if (strcmp(format, "VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM") == 0) {
			return VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM;
		}
		if (strcmp(format, "VK_FORMAT_R10X6_UNORM_PACK16") == 0) {
			return VK_FORMAT_R10X6_UNORM_PACK16;
		}
		if (strcmp(format, "VK_FORMAT_R10X6G10X6_UNORM_2PACK16") == 0) {
			return VK_FORMAT_R10X6G10X6_UNORM_2PACK16;
		}
		if (strcmp(format, "VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16") == 0) {
			return VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16;
		}
		if (strcmp(format, "VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16") == 0) {
			return VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16;
		}
		if (strcmp(format, "VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16") == 0) {
			return VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16;
		}
		if (strcmp(format, "VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16") == 0) {
			return VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16;
		}
		if (strcmp(format, "VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16") == 0) {
			return VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16;
		}
		if (strcmp(format, "VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16") == 0) {
			return VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16;
		}
		if (strcmp(format, "VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16") == 0) {
			return VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16;
		}
		if (strcmp(format, "VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16") == 0) {
			return VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16;
		}
		if (strcmp(format, "VK_FORMAT_R12X4_UNORM_PACK16") == 0) {
			return VK_FORMAT_R12X4_UNORM_PACK16;
		}
		if (strcmp(format, "VK_FORMAT_R12X4G12X4_UNORM_2PACK16") == 0) {
			return VK_FORMAT_R12X4G12X4_UNORM_2PACK16;
		}
		if (strcmp(format, "VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16") == 0) {
			return VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16;
		}
		if (strcmp(format, "VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16") == 0) {
			return VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16;
		}
		if (strcmp(format, "VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16") == 0) {
			return VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16;
		}
		if (strcmp(format, "VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16") == 0) {
			return VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16;
		}
		if (strcmp(format, "VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16") == 0) {
			return VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16;
		}
		if (strcmp(format, "VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16") == 0) {
			return VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16;
		}
		if (strcmp(format, "VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16") == 0) {
			return VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16;
		}
		if (strcmp(format, "VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16") == 0) {
			return VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16;
		}
		if (strcmp(format, "VK_FORMAT_G16B16G16R16_422_UNORM") == 0) {
			return VK_FORMAT_G16B16G16R16_422_UNORM;
		}
		if (strcmp(format, "VK_FORMAT_B16G16R16G16_422_UNORM") == 0) {
			return VK_FORMAT_B16G16R16G16_422_UNORM;
		}
		if (strcmp(format, "VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM") == 0) {
			return VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM;
		}
		if (strcmp(format, "VK_FORMAT_G16_B16R16_2PLANE_420_UNORM") == 0) {
			return VK_FORMAT_G16_B16R16_2PLANE_420_UNORM;
		}
		if (strcmp(format, "VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM") == 0) {
			return VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM;
		}
		if (strcmp(format, "VK_FORMAT_G16_B16R16_2PLANE_422_UNORM") == 0) {
			return VK_FORMAT_G16_B16R16_2PLANE_422_UNORM;
		}
		if (strcmp(format, "VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM") == 0) {
			return VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM;
		}
		if (strcmp(format, "VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG") == 0) {
			return VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG;
		}
		if (strcmp(format, "VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG") == 0) {
			return VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG;
		}
		if (strcmp(format, "VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG") == 0) {
			return VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG;
		}
		if (strcmp(format, "VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG") == 0) {
			return VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG;
		}
		if (strcmp(format, "VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG") == 0) {
			return VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG;
		}
		if (strcmp(format, "VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG") == 0) {
			return VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG;
		}
		if (strcmp(format, "VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG") == 0) {
			return VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG;
		}
		if (strcmp(format, "VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG") == 0) {
			return VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG;
		}
		if (strcmp(format, "VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK_EXT") == 0) {
			return VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK_EXT;
		}
		if (strcmp(format, "VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK_EXT") == 0) {
			return VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK_EXT;
		}
		if (strcmp(format, "VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK_EXT") == 0) {
			return VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK_EXT;
		}
		if (strcmp(format, "VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK_EXT") == 0) {
			return VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK_EXT;
		}
		if (strcmp(format, "VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK_EXT") == 0) {
			return VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK_EXT;
		}
		if (strcmp(format, "VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK_EXT") == 0) {
			return VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK_EXT;
		}
		if (strcmp(format, "VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK_EXT") == 0) {
			return VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK_EXT;
		}
		if (strcmp(format, "VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK_EXT") == 0) {
			return VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK_EXT;
		}
		if (strcmp(format, "VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK_EXT") == 0) {
			return VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK_EXT;
		}
		if (strcmp(format, "VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK_EXT") == 0) {
			return VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK_EXT;
		}
		if (strcmp(format, "VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK_EXT") == 0) {
			return VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK_EXT;
		}
		if (strcmp(format, "VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK_EXT") == 0) {
			return VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK_EXT;
		}
		if (strcmp(format, "VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK_EXT") == 0) {
			return VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK_EXT;
		}
		if (strcmp(format, "VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK_EXT") == 0) {
			return VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK_EXT;
		}
		if (strcmp(format, "VK_FORMAT_G8_B8R8_2PLANE_444_UNORM_EXT") == 0) {
			return VK_FORMAT_G8_B8R8_2PLANE_444_UNORM_EXT;
		}
		if (strcmp(format, "VK_FORMAT_G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16_EXT") == 0) {
			return VK_FORMAT_G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16_EXT;
		}
		if (strcmp(format, "VK_FORMAT_G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16_EXT") == 0) {
			return VK_FORMAT_G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16_EXT;
		}
		if (strcmp(format, "VK_FORMAT_G16_B16R16_2PLANE_444_UNORM_EXT") == 0) {
			return VK_FORMAT_G16_B16R16_2PLANE_444_UNORM_EXT;
		}
		if (strcmp(format, "VK_FORMAT_A4R4G4B4_UNORM_PACK16_EXT") == 0) {
			return VK_FORMAT_A4R4G4B4_UNORM_PACK16_EXT;
		}
		if (strcmp(format, "VK_FORMAT_A4B4G4R4_UNORM_PACK16_EXT") == 0) {
			return VK_FORMAT_A4B4G4R4_UNORM_PACK16_EXT;
		}

		return VK_FORMAT_UNDEFINED;
	}

	RenderPassOperation RenderPassOperationFromString(char const* op) {
		if (strcmp(op, "VK_ATTACHMENT_LOAD_OP_CLEAR") == 0) {
			return RenderPassOperation::Clear;
		}
		else if (strcmp(op, "VK_ATTACHMENT_LOAD_OP_LOAD") == 0) {
			return RenderPassOperation::Load;
		}

		return RenderPassOperation::DontCare;
	}

	/*static  std::unordered_map<std::string, FrameGraphResourceType> const FrameGraphResourceTypeTable = {
		{"buffer", FrameGraphResourceType_Buffer},
		{"texture", FrameGraphResourceType_Texture},
		{"attachment", FrameGraphResourceType_Attachment},
		{"reference",FrameGraphResourceType_Reference}
	};*/


	//static FrameGraphResourceType GetResourceType(std::string id) {
	//	//Handle lower/upper case
	//	return FrameGraphResourceTypeTable.find(id)->second;
	//}

#pragma endregion

	static bool IsNodeDependant(FrameGraph* graph, FrameGraphNodeHandle childNode, FrameGraphNodeHandle targetNode) {

		if (childNode.index == targetNode.index)
		{
			return false;
		}

		auto n = graph->GetNode(childNode);

		for (auto inputEdge : n->inputEdges) {

			auto edge = graph->GetNodeEdge(inputEdge);
			if (edge->from.index == targetNode.index)
			{
				return true;
			}

			return IsNodeDependant(graph, edge->from, targetNode);
		}

		return false;
	}

	static void ComputeEdges(FrameGraph* frameGraph)
	{
		for (size_t i = 0; i < frameGraph->nodes.size(); i++)
		{
			auto nodeHandle = (FrameGraphNodeHandle)i;
			auto node = frameGraph->GetNode(nodeHandle);
			if (i == 0)
				continue;


			for (int j = i - 1; j >= 0; j--)
			{
				auto parentNodeHandle = (FrameGraphNodeHandle)j;
				auto parentNode = frameGraph->GetNode(parentNodeHandle);

				for (auto parentOutput : parentNode->outputs) {
					for (auto input : node->inputs) {
						if (input.resource->Handle().IsEqual(parentOutput.resource->Handle()))
						{
							//Only create edge if these nodes are not already related in the dependency chain
							if (!IsNodeDependant(frameGraph, node->nodeHandle, parentNode->nodeHandle))
							{
								auto edgeHandle = frameGraph->builder->CreateNodeEdge(parentNodeHandle, nodeHandle, input.resource->Handle());
								parentNode->outputEdges.push_back(edgeHandle);
								node->inputEdges.push_back(edgeHandle);

								auto edge = frameGraph->GetNodeEdge(edgeHandle);

								std::printf("Edge: {%s} {%i}->{%s} {%i} \n", parentNode->name.c_str(), edge->from, node->name.c_str(), edge->to);
							}
						}
					}
				}
			}
		}
	}


	void FrameGraphBuilder::Init(PuduGraphics* device)
	{
		this->graphics = device;
		resourceCache.Init(device);
		nodeCache.Init(device);
		renderPassCache.Init();
	}
	void FrameGraphBuilder::Shutdown()
	{
	}

	/// <summary>
	/// Deprecated, all resources should be allocated externally
	/// </summary>
	/// <param name="creation"></param>
	/// <returns></returns>
	//GPUResourceHandle FrameGraphBuilder::CreateOrGetFrameGraphResource(const FrameGraphResourceCreateInfo& creation)
	//{
	//	auto r = GetResource(creation.name);
	//	if (r != nullptr)
	//	{
	//		return r->resourceHandle;
	//	}

	//	GPUResourceHandle resourceHandle{ k_INVALID_HANDLE };
	//	resourceHandle.index = resourceCache.resources.ObtainResource();

	//	GPUResource* resource = resourceCache.resources.GetResourcePtr(resourceHandle.index);
	//	/*	resource->name.append(creation.name);
	//		resource->type = creation.type;
	//		resource->isDepth = creation.isDepth;*/

	//		//If node is not a ref, then we'll need to create it
	//	if (creation.type != FrameGraphResourceType_Reference)
	//	{
	//		//Copy from resourceInfo
	//		resource->name = creation.name;
	//		resource->type = creation.type;
	//		resource->textureHandle = creation.textureHandle;
	//		resource->buffer = creation.buffer;
	//		resource->external = creation.external;
	//		resource->size = creation.size;
	//		resource->flags = creation.flags;
	//		resource->width = creation.width;
	//		resource->height = creation.height;
	//		resource->loadOp = creation.loadOp;
	//		resource->textureType = creation.textureType;
	//		resource->format = creation.format;
	//		resource->sizeType = creation.sizeType;
	//		resource->depth = creation.depth;
	//		resource->allocated = false;


	//		resource->resourceHandle = resourceHandle;

	//		//Add to resource cache
	//		resourceCache.AddResourceToCache(creation.name.c_str(), resourceHandle);
	//	}

	//	return resourceHandle;
	//}

	FrameGraphNodeHandle FrameGraphBuilder::CreateNode(const FrameGraphNodeCreation& creation)
	{
		FrameGraphNodeHandle nodeHandle{ k_INVALID_HANDLE };
		nodeHandle.index = nodeCache.nodes.ObtainResource();

		FrameGraphNode* node = nodeCache.nodes.GetResourcePtr(nodeHandle.index);

		node->name.append(creation.name);
		node->enabled = creation.enabled;
		node->inputs = creation.inputs;
		node->outputs = creation.outputs;
		//node->outputEdges.reserve(creation.outputs.size());
		node->framebuffer = { k_INVALID_HANDLE };
		node->renderPass = creation.renderPass;
		//node->type = creation.renderType;
		node->isCompute = creation.isCompute;
		node->nodeHandle = nodeHandle;

		nodeCache.nodeMap[node->name] = nodeHandle.index;

		/*	for (size_t i = 0; i < creation.outputs.size(); ++i) {
				const FrameGraphResourceCreateInfo& outputCreation = creation.outputs[i];

				FrameGraphResourceHandle output = CreateOrGetFrameGraphResource(outputCreation);

				node->outputs.push_back(output);
			}

			for (size_t i = 0; i < creation.inputs.size(); ++i) {
				const FrameGraphResourceCreateInfo& input_creation = creation.inputs[i];

				FrameGraphResourceHandle input_handle = CreateOrGetFrameGraphResource(input_creation);

				node->inputs.push_back(input_handle);
			}*/

		return nodeHandle;
	}

	FrameGraphNode* FrameGraphBuilder::GetNode(std::string name)
	{
		auto nodeIt = nodeCache.nodeMap.find(name);

		if (nodeIt == nodeCache.nodeMap.end())
		{
			return nullptr;
		}

		FrameGraphNode* node = nodeCache.nodes.GetResourcePtr(nodeIt->second);

		return node;
	}

	FrameGraphNode* FrameGraphBuilder::GetNode(FrameGraphNodeHandle nodeHandle)
	{
		return nodeCache.nodes.GetResourcePtr(nodeHandle.index);
	}

	GPUResource* FrameGraphBuilder::GetResource(std::string name)
	{
		auto it = resourceCache.resourcesMap.find(name);

		if (it == resourceCache.resourcesMap.end())
		{
			return nullptr;
		}

		return resourceCache.resources.GetResourcePtr(it->second);
	}

	GPUResource* FrameGraphBuilder::GetResource(GPUResourceHandle textureHandle)
	{
		//This could also be a buffer, we will need more information about what type of handle it is
		return graphics->Resources()->GetTexture<RenderTexture>(textureHandle).get();
	}

	NodeEdge* FrameGraphBuilder::GetNodeEdge(NodeEdgeHandle textureHandle)
	{
		return nodeCache.nodeEdges.GetResourcePtr(textureHandle.index);
	}

	NodeEdgeHandle FrameGraphBuilder::CreateNodeEdge(FrameGraphNodeHandle from, FrameGraphNodeHandle to, GPUResourceHandle resourceHandle)
	{
		NodeEdgeHandle edgeHandle{ k_INVALID_HANDLE };
		edgeHandle.index = nodeCache.nodeEdges.ObtainResource();

		NodeEdge* edge = nodeCache.nodeEdges.GetResourcePtr(edgeHandle.index);

		edge->to = to;
		edge->from = from;
		edge->resource = resourceHandle;
		edge->handle = edgeHandle;


		return edgeHandle;
	}

	void FrameGraphResourceCache::Init(PuduGraphics* device)
	{
		this->device = device;
	}

	void FrameGraphResourceCache::Shutdown()
	{
	}

	bool FrameGraphResourceCache::AddResourceToCache(const char* name, GPUResourceHandle resourceHandle)
	{
		if (resourcesMap.find(name) != resourcesMap.end())
		{
			resourcesMap[name] = resourceHandle.index;

			return true;
		}
		return false;
	}

	void FrameGraphNodeCache::Init(PuduGraphics* device)
	{
		this->device = device;
	}

	void FrameGraphNodeCache::Shutdown()
	{

	}
	void FrameGraph::Init(FrameGraphBuilder* builder)
	{
		this->builder = builder;
		nodes.reserve(FrameGraphBuilder::K_MAX_NODES_COUNT);
	}


	//void FrameGraph::Parse(std::filesystem::path filePath)
	//{
	//	LOG("FrameGraph Parse");
	//	//TODO: ADD NODE TYPE (COLOR,DEPTH)
	//	if (!std::filesystem::exists(filePath))
	//	{
	//		LOG("File not found {}", filePath.string());
	//	}

	//	using namespace simdjson;

	//	ondemand::parser parser;

	//	auto json = padded_string::load(filePath.string());
	//	auto doc = parser.iterate(json);
	//	ondemand::object object = doc.get_object();
	//	for (auto field : object)
	//	{
	//		std::string_view keyv = field.unescaped_key();
	//	}

	//	auto passes = object["passes"];
	//	for (auto pass : passes)
	//	{
	//		FrameGraphNodeCreation nodeCreation;
	//		auto name = pass["name"].get_string();
	//		std::string nameString(name.value());
	//		nodeCreation.name = nameString.c_str();

	//		auto family = pass["family"].get_string();

	//		if (family.error() == simdjson::error_code::SUCCESS)
	//		{
	//			if (family.value().compare("compute") == 0)
	//			{
	//				nodeCreation.isCompute = true;
	//			}
	//		}

	//		nodeCreation.enabled = pass["enabled"].get_bool();
	//		nodeCreation.renderType = GetRenderPassType(std::string(pass["type"].get_string().value())); //We need to store the string_view into a string

	//		auto passInputs = pass["inputs"];

	//		for (auto input : passInputs)
	//		{
	//			FrameGraphResourceCreateInfo inputCreation{};

	//			inputCreation.type = GetResourceType(std::string(input["type"].get_string().value()));
	//			auto inputName = input["name"].get_string();
	//			auto isDepthNode = input["isDepth"];
	//			inputCreation.isDepth = false;

	//			if (isDepthNode.error() == simdjson::error_code::SUCCESS)
	//			{
	//				inputCreation.isDepth = isDepthNode.get_bool();
	//			}

	//			std::string inputNameStr;
	//			inputNameStr.append(inputName.value());
	//			inputCreation.name = inputNameStr;

	//			inputCreation.external = false;

	//			//	nodeCreation.inputs.push_back(inputCreation);
	//		}

	//		auto passOutputs = pass["outputs"];

	//		for (auto output : passOutputs)
	//		{
	//			FrameGraphResourceCreateInfo outputCreation{};
	//			outputCreation.type = GetResourceType(std::string(output["type"].get_string().value()).c_str());
	//			auto outputName = output["name"].get_string();
	//			std::string outputNameStr;
	//			outputNameStr.append(outputName.value());
	//			outputCreation.name = outputNameStr;

	//			switch (outputCreation.type)
	//			{
	//			case FrameGraphResourceType_Attachment:
	//			case FrameGraphResourceType_Texture:
	//			{
	//				std::string format = std::string(output["format"].get_string().value());

	//				outputCreation.format = VkFormatFromString(format.c_str());

	//				std::string loadOp = std::string(output["op"].get_string().value());

	//				outputCreation.loadOp = RenderPassOperationFromString(loadOp.c_str());


	//				auto textureType = output["textureType"].get_string();

	//				if (textureType.error() == simdjson::error_code::SUCCESS)
	//				{
	//					outputCreation.textureType = TextureTypeFromString(textureType.value().data());
	//				}
	//				else {
	//					outputCreation.textureType = TextureType::Texture2D;
	//				}

	//				auto resolution = output["resolution"].get_array();
	//				std::vector<uint32_t> values;
	//				for (auto r : resolution)
	//				{
	//					values.push_back((uint32_t)r.get_int64());
	//				}

	//				outputCreation.textureHandle = { k_INVALID_HANDLE };
	//				outputCreation.width = values[0];
	//				outputCreation.height = values[1];
	//				outputCreation.depth = 1;

	//			}break;
	//			case FrameGraphResourceType_Buffer:
	//			{
	//				//TODO
	//			}
	//			break;


	//			default:
	//				break;
	//			}

	//			//nodeCreation.outputs.push_back(outputCreation);
	//		}

	//		FrameGraphNodeHandle nodeHandle = builder->CreateNode(nodeCreation);

	//		nodes.push_back(nodeHandle);
	//	}

	//	LOG("FrameGraph Parse End");
	//}
	void FrameGraph::Reset()
	{
		//Todo:: implement
	}

	void FrameGraph::AllocateResource(GPUResourceHandle handle)
	{
		auto r = builder->GetResource(handle);

		if (r->IsAllocated())
		{
			return;
		}

		LOG("Allocating {}", r->name);


		r->Create(builder->graphics);
	}

	void FrameGraph::AllocateRequiredResources()
	{
		LOG("FrameGraph: Allocating Resources");
		for (auto nodeHandle : nodes)
		{
			auto node = builder->GetNode(nodeHandle);

			for (auto outputResource : node->outputs)
			{
				AllocateResource(outputResource.resource->Handle());
			}

			for (auto inputResource : node->inputs)
			{
				AllocateResource(inputResource.resource->Handle());
			}
		}
		LOG("FrameGraph: Allocating Resources End");
	}

	void FrameGraph::EnableRenderPass(char* renderPassName)
	{
		builder->GetNode(renderPassName)->enabled = true;
	}
	void FrameGraph::DisableRenderPass(char* renderPassName)
	{
		builder->GetNode(renderPassName)->enabled = false;
	}
	void FrameGraph::Compile()
	{
		LOG("FrameGraph Compile");

		ComputeEdges(this);

		/*	for (auto nodeHande : nodes) {
				FrameGraphNode* node = builder->GetNode(nodeHande);

				node->outputEdges.clear();
			}*/

			//for (auto nodeHandle : nodes) {
			//	FrameGraphNode* node = builder->GetNode(nodeHandle);

			//	if (!node->enabled)
			//	{
			//		continue;
			//	}

			//	//	ComputeEdges(this, node, nodeHandle);
			//}

			//Sorted nodes in reverse order
		std::vector<FrameGraphNodeHandle> sortedNodes;
		sortedNodes.reserve(nodes.size());

		std::vector<uint16_t> visited;
		visited.resize(nodes.size());

		std::vector<FrameGraphNodeHandle> nodesToBeVisitedStack;
		nodesToBeVisitedStack.reserve(nodes.size());

		//Topological Sorting
		for (FrameGraphNodeHandle nodeHandle : nodes)
		{
			FrameGraphNode* node = builder->GetNode(nodeHandle);
			if (!node->enabled)
			{
				continue;
			}

			nodesToBeVisitedStack.push_back(nodeHandle);

			while (nodesToBeVisitedStack.size() > 0)
			{
				FrameGraphNodeHandle nodeToVisitHandle = nodesToBeVisitedStack.back();

				if (visited[nodeToVisitHandle.index] == 2)
				{
					nodesToBeVisitedStack.pop_back();
					continue;
				}

				if (visited[nodeToVisitHandle.index] == 1)
				{
					visited[nodeToVisitHandle.index] = 2;

					sortedNodes.push_back(nodeToVisitHandle);

					nodesToBeVisitedStack.pop_back();

					continue;
				}

				visited[nodeToVisitHandle.index] = 1; //Mark as visited

				if (node->outputEdges.size() == 0) {
					continue; //Leaf node, nothing to do here
				}

				//Add child handles
				for (auto edgeHandle : node->outputEdges)
				{
					auto edge = GetNodeEdge(edgeHandle);

					if (!visited[edge->to.index])
					{
						nodesToBeVisitedStack.push_back(edge->to);
					}
				}
			}
		}

		nodes.clear();

		for (int32_t i = sortedNodes.size() - 1; i >= 0; i--)
		{
			nodes.push_back(sortedNodes[i]);
		}

		for (auto nodeHandle : nodes)
		{
			auto node = builder->GetNode(nodeHandle);

			if (!node->enabled)
			{
				continue;
			}
		}

		LOG("FrameGraph Compile End");
	}
	void FrameGraph::RenderFrame(RenderFrameData& renderData)
	{
		auto commands = renderData.currentCommand;
		auto gfx = renderData.graphics;

		for (auto nodeHandle : nodes)
		{
			//TODO: PUT MARKERS
			FrameGraphNode* node = builder->GetNode(nodeHandle);
			auto renderPass = gfx->Resources()->GetRenderPass(node->renderPass);

			if (!renderPass->isEnabled)
			{
				continue;
			}
			commands->Clear(vec4(0.2, 0.2, 0.3, 1.0));
			commands->ClearDepthStencil(1.0f, 0);

			uint16_t width = 0;
			uint16_t height = 0;

			renderData.currentRenderPass = renderPass;


			for (auto& inputResource : node->inputs)
			{
				if (inputResource.type == GPUResourceType::Texture)
				{
					auto texture = gfx->Resources()->GetTexture<RenderTexture>(inputResource.resource->Handle());

					width = texture->width;
					height = texture->height;
				}
			}

			for (auto& outputResource : node->outputs)
			{
				if (outputResource.type == GPUResourceType::Texture)
				{
					auto texture = gfx->Resources()->GetTexture<Texture2d>(outputResource.resource->Handle());

					width = texture->width;
					height = texture->height;

					if (TextureFormat::HasDepth(texture->format))
					{
						commands->AddImageBarrier(texture->vkImageHandle, RESOURCE_STATE_UNDEFINED, RESOURCE_STATE_DEPTH_WRITE, 0, 1, true);
					}
					else
					{
						commands->AddImageBarrier(texture->vkImageHandle, RESOURCE_STATE_UNDEFINED, RESOURCE_STATE_RENDER_TARGET, 0, 1, false);
					}
				}
			}

			commands->SetDepthBias(0, 0);
			commands->SetScissor(0, 0, width, height);
			commands->SetViewport({ {0,0,width,height},0,1 });
			renderData.width = width;
			renderData.height = height;

			/*auto graphRenderPass = renderData.m_renderPassesByType->find(node->type)->second;*/

			renderData.activeRenderTarget = gfx->Resources()->GetTexture<RenderTexture>(node->outputs[0].resource->Handle());
			renderPass->PreRender(renderData);
			renderPass->BeginRender(renderData);
			renderPass->Render(renderData);
			renderPass->EndRender(renderData);
			renderPass->AfterRender(renderData);
			//TODO: IMPLEMENT MARKERS
		}
	}
	void FrameGraph::OnResize(PuduGraphics& gpu, uint32_t new_width, uint32_t new_height)
	{
	}
	std::string FrameGraph::ToString()
	{
		std::stringstream ss;

		for (auto n : nodes)
		{
			auto node = GetNode(n);

			for (auto edge : node->outputEdges) {
				ss << node->name << "-->" << GetNode(GetNodeEdge(edge)->to)->name << "\n";
			}
		}

		return ss.str();
	}
	void FrameGraph::AttachRenderPass(RenderPass renderPass, RenderPassType type)
	{

	}

	FrameGraphNode* FrameGraph::GetNode(char* name)
	{
		return builder->GetNode(name);
	}
	FrameGraphNode* FrameGraph::GetNode(FrameGraphNodeHandle nodeHandle)
	{
		return builder->GetNode(nodeHandle);
	}
	NodeEdge* FrameGraph::GetNodeEdge(NodeEdgeHandle edgeHandle)
	{
		return builder->GetNodeEdge(edgeHandle);
	}

	FrameGraphNodeHandle FrameGraph::CreateNode(FrameGraphNodeCreation& creationData)
	{
		auto renderPass = builder->graphics->Resources()->GetRenderPass(creationData.renderPass);

		for (size_t i = 0; i < renderPass->attachments.colorAttachmentCount; i++) {

			auto attachment = renderPass->attachments.colorAttachments[i];
			if (attachment.storeOp == VK_ATTACHMENT_STORE_OP_STORE)
			{
				creationData.outputs.push_back(attachment);
			}

			creationData.inputs.push_back(attachment);
		}

		if (renderPass->attachments.depthAttachmentCount > 0)
		{
			auto attachment = renderPass->attachments.depthAttachments[0];
			creationData.inputs.push_back(attachment);

			if (attachment.storeOp == VK_ATTACHMENT_STORE_OP_STORE)
			{
				creationData.outputs.push_back(attachment);
			}
		}

		auto handle = builder->CreateNode(creationData);
		nodes.push_back(handle);

		return handle;
	}

	void FrameGraphRenderPassCache::Init()
	{
	}
	void FrameGraphRenderPassCache::Shutdown()
	{
	}
	VkAttachmentLoadOp GetVkAttachmentLoadOp(RenderPassOperation op)
	{
		switch (op)
		{
		case Pudu::DontCare:
			return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			break;
		case Pudu::Load:
			return VK_ATTACHMENT_LOAD_OP_LOAD;
			break;
		case Pudu::Clear:
			return VK_ATTACHMENT_LOAD_OP_CLEAR;
			break;
		case Pudu::Count:
			return VK_ATTACHMENT_LOAD_OP_NONE_EXT;
			break;
		default:
			return VK_ATTACHMENT_LOAD_OP_NONE_EXT;
			break;
		}
	}
}
