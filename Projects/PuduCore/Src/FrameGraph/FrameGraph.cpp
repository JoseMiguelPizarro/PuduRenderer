#include <filesystem>
#include <simdjson.h>
#include "Logger.h"
#include "FrameGraph/FrameGraph.h"
#include <Resources/RenderPassCreationData.h>
#include <Resources/FrameBufferCreationData.h>
#include "FrameGraph/FrameGraphRenderPass.h"

namespace Pudu
{

#pragma region Utils
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

	static  std::unordered_map<std::string, FrameGraphResourceType> const FrameGraphResourceTypeTable = {
		{"buffer", FrameGraphResourceType_Buffer},
		{"texture", FrameGraphResourceType_Texture},
		{"attachment", FrameGraphResourceType_Attachment},
		{"reference",FrameGraphResourceType_Reference}
	};

	static std::unordered_map < std::string, RenderPassType> const RenderPassTypeTable =
	{
		{"color", RenderPassType::Color},
		{"depth", RenderPassType::DepthPrePass}
	};

	static RenderPassType GetRenderPassType(char const* id) {
		return RenderPassTypeTable.find(id)->second;
	}

	static FrameGraphResourceType GetResourceType(char const* id) {
		//Handle lower/upper case
		return FrameGraphResourceTypeTable.find(id)->second;
	}

#pragma endregion

	static void ComputeEdges(FrameGraph* frameGraph, FrameGraphNode* node, FrameGraphNodeHandle nodeHandle)
	{
		LOG("Compute Edges");
		for (FrameGraphResourceHandle nodeInput : node->inputs)
		{
			FrameGraphResource* inputResource = frameGraph->GetResource(nodeInput);

			FrameGraphResource* outputResource = frameGraph->GetOutputResource(inputResource->name.c_str());
			if (outputResource == nullptr && !inputResource->resourceInfo.external) {
				continue;
			}

			inputResource->producer = outputResource->producer;
			inputResource->resourceInfo = outputResource->resourceInfo;
			inputResource->outputHandle = outputResource->outputHandle;

			FrameGraphNode* parentNode = frameGraph->GetNode(inputResource->producer);

			parentNode->edges.push_back(nodeHandle);
		}
		LOG("Compute Edges End");
	}

	static void CreateRenderPass(FrameGraph* frameGraph, FrameGraphNode* node) {
		LOG("FrameGraph: Create RenderPass");
		RenderPassCreationData renderPassCreation{ };
		renderPassCreation.SetName(node->name.c_str());

		for (auto outputResourceHandle : node->outputs) {
			FrameGraphResource* outputResource = frameGraph->GetResource(outputResourceHandle);

			FrameGraphResourceInfo& info = outputResource->resourceInfo;

			if (outputResource->type == FrameGraphResourceType_Attachment) {
				if (info.texture.format == VK_FORMAT_D32_SFLOAT) {
					renderPassCreation.SetDepthStencilTexture(info.texture.format, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
					renderPassCreation.depthOperation = RenderPassOperation::Clear;
				}
				else {
					renderPassCreation.AddAttachment(info.texture.format, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, info.texture.loadOp);
				}
			}
		}

		for (auto inputResourceHandle : node->inputs) {
			FrameGraphResource* inputResource = frameGraph->GetResource(inputResourceHandle);

			FrameGraphResourceInfo& info = inputResource->resourceInfo;

			if (inputResource->type == FrameGraphResourceType_Attachment) {
				if (info.texture.format == VK_FORMAT_D32_SFLOAT) {
					renderPassCreation.SetDepthStencilTexture(info.texture.format, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

					renderPassCreation.depthOperation = RenderPassOperation::Load;
				}
				else {
					renderPassCreation.AddAttachment(info.texture.format, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, RenderPassOperation::Load);
				}
			}
		}

		auto renderPassHandle = frameGraph->builder->graphics->Resources()->AllocateRenderPass(renderPassCreation);
		node->renderPass = renderPassHandle;

		LOG("FrameGraph: Create RenderPass End");
	}

	static void CreateFrameBuffer(FrameGraph* frameGraph, FrameGraphNode* node) {
		FramebufferCreationData framebufferCreationData{ };
		framebufferCreationData.renderPassHandle = node->renderPass;
		framebufferCreationData.SetName(node->name.c_str());

		uint32_t width = 0;
		uint32_t height = 0;

		for (auto outputResourceHandle : node->outputs) {
			FrameGraphResource* resource = frameGraph->GetResource(outputResourceHandle);

			FrameGraphResourceInfo& info = resource->resourceInfo;

			if (resource->type == FrameGraphResourceType_Buffer || resource->type == FrameGraphResourceType_Reference) {
				continue;
			}

			width = info.texture.width;
			height = info.texture.height;

			if (info.texture.format == VK_FORMAT_D32_SFLOAT) {
				framebufferCreationData.SetDepthStencilTexture(info.texture.handle);
			}
			else
			{
				framebufferCreationData.AddRenderTexture(info.texture.handle);
			}
		}

		for (auto inputResourceHandle : node->inputs) {
			FrameGraphResource* inputResource = frameGraph->GetResource(inputResourceHandle);

			if (inputResource->type == FrameGraphResourceType_Buffer || inputResource->type == FrameGraphResourceType_Reference) {
				continue;
			}

			FrameGraphResource* resource = frameGraph->GetOutputResource(inputResource->name.c_str());

			FrameGraphResourceInfo& info = resource->resourceInfo;

			inputResource->resourceInfo.texture.handle = info.texture.handle;

			width = info.texture.width;
			height = info.texture.height;

			if (inputResource->type == FrameGraphResourceType_Texture) {
				continue;
			}

			if (info.texture.format == VK_FORMAT_D32_SFLOAT) {
				framebufferCreationData.SetDepthStencilTexture(info.texture.handle);
			}
			else {
				framebufferCreationData.AddRenderTexture(info.texture.handle);
			}
		}

		framebufferCreationData.width = width;
		framebufferCreationData.height = height;
		auto frameBufferHandle = frameGraph->builder->graphics->Resources()->AllocateFrameBuffer(framebufferCreationData);
		auto frameBuffer = frameGraph->builder->graphics->Resources()->GetFramebuffer(frameBufferHandle);

		node->framebuffer = frameBufferHandle;
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

	FrameGraphResourceHandle FrameGraphBuilder::CreateNodeOutputResource(const FrameGraphResourceOutputCreation& creation, FrameGraphNodeHandle producer)
	{
		FrameGraphResourceHandle resourceHandle{ k_INVALID_HANDLE };

		resourceHandle.index = resourceCache.resources.ObtainResource();

		FrameGraphResource* resource = resourceCache.resources.GetResourcePtr(resourceHandle.index);
		resource->name.append(creation.name);
		resource->type = creation.type;

		//If node is not a ref, then we'll need to create it
		if (creation.type != FrameGraphResourceType_Reference)
		{
			resource->resourceInfo = creation.resourceInfo;
			resource->outputHandle = resourceHandle;
			resource->producer = producer;
			resource->RefCount = 0;

			//Add to resource cache
			resourceCache.resourcesMap[resource->name.c_str()] = resourceHandle.index;
		}

		return resourceHandle;
	}

	FrameGraphResourceHandle FrameGraphBuilder::CreateNodeInput(const FrameGraphResourceInputCreation& creation)
	{
		FrameGraphResourceHandle resourceHandle = { k_INVALID_HANDLE };

		resourceHandle.index = resourceCache.resources.ObtainResource();

		FrameGraphResource* resource = resourceCache.resources.GetResourcePtr(resourceHandle.index);

		resource->type = creation.type;
		resource->name = creation.name;

		resource->resourceInfo = {};
		resource->producer.index = k_INVALID_HANDLE;
		resource->outputHandle.index = k_INVALID_HANDLE;
		resource->RefCount = 0;

		return resourceHandle;
	}

	FrameGraphNodeHandle FrameGraphBuilder::CreateNode(const FrameGraphNodeCreation& creation)
	{
		FrameGraphNodeHandle nodeHandle{ k_INVALID_HANDLE };
		nodeHandle.index = nodeCache.nodes.ObtainResource();

		FrameGraphNode* node = nodeCache.nodes.GetResourcePtr(nodeHandle.index);

		node->name.append(creation.name);
		node->enabled = creation.enabled;
		node->inputs.reserve(creation.inputs.size());
		node->outputs.reserve(creation.outputs.size());
		node->edges.reserve(creation.outputs.size());
		node->framebuffer = { k_INVALID_HANDLE };
		node->renderPass = { k_INVALID_HANDLE };

		nodeCache.nodeMap[node->name] = nodeHandle.index;

		for (size_t i = 0; i < creation.outputs.size(); ++i) {
			const FrameGraphResourceOutputCreation& outputCreation = creation.outputs[i];

			FrameGraphResourceHandle output = CreateNodeOutputResource(outputCreation, nodeHandle);

			node->outputs.push_back(output);
		}

		for (size_t i = 0; i < creation.inputs.size(); ++i) {
			const FrameGraphResourceInputCreation& input_creation = creation.inputs[i];

			FrameGraphResourceHandle input_handle = CreateNodeInput(input_creation);

			node->inputs.push_back(input_handle);
		}

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

	FrameGraphNode* FrameGraphBuilder::GetNode(FrameGraphNodeHandle handle)
	{
		return nodeCache.nodes.GetResourcePtr(handle.index);
	}

	FrameGraphResource* FrameGraphBuilder::GetOutputResource(std::string name)
	{
		auto it = resourceCache.resourcesMap.find(name);

		if (it == resourceCache.resourcesMap.end())
		{
			return nullptr;
		}

		return resourceCache.resources.GetResourcePtr(it->second);
	}

	FrameGraphResource* FrameGraphBuilder::GetResource(FrameGraphResourceHandle handle)
	{
		return resourceCache.resources.GetResourcePtr(handle.index);
	}

	void FrameGraphResourceCache::Init(PuduGraphics* device)
	{
		this->device = device;
	}

	void FrameGraphResourceCache::Shutdown()
	{
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

	void FrameGraph::Shutdown()
	{
		for (uint32_t i = 0; i < nodes.size(); i++)
		{
			FrameGraphNode* node = builder->GetNode(nodes[i]);

			builder->graphics->DestroyRenderPass(node->renderPass);
			builder->graphics->DestroyFrameBuffer(node->framebuffer);
		}
	}
	void FrameGraph::Parse(std::filesystem::path filePath)
	{
		LOG("FrameGraph Parse");
		//TODO: ADD NODE TYPE (COLOR,DEPTH)
		if (!std::filesystem::exists(filePath))
		{
			LOG("File not found {}", filePath.string());
		}

		using namespace simdjson;

		ondemand::parser parser;

		auto json = padded_string::load(filePath.string());
		auto doc = parser.iterate(json);
		ondemand::object object = doc.get_object();
		for (auto field : object)
		{
			std::string_view keyv = field.unescaped_key();
		}

		auto passes = object["passes"];
		for (auto pass : passes)
		{
			FrameGraphNodeCreation nodeCreation;
			auto name = pass["name"].get_string();
			std::string nameString(name.value());
			nodeCreation.name = nameString.c_str();

			nodeCreation.enabled = pass["enabled"].get_bool();
			nodeCreation.renderType = GetRenderPassType(std::string(pass["type"].get_string().value()).c_str()); //We need to store the string_view into a string

			auto passInputs = pass["inputs"];

			for (auto input : passInputs)
			{
				FrameGraphResourceInputCreation inputCreation{};

				inputCreation.type = GetResourceType(std::string(input["type"].get_string().value()).c_str());
				auto inputName = input["name"].get_string();
				std::string inputNameStr;
				inputNameStr.append(inputName.value());
				inputCreation.name = inputNameStr;

				inputCreation.resource_info.external = false;

				nodeCreation.inputs.push_back(inputCreation);
			}

			auto passOutputs = pass["outputs"];

			for (auto output : passOutputs)
			{
				FrameGraphResourceOutputCreation outputCreation{};
				outputCreation.type = GetResourceType(std::string(output["type"].get_string().value()).c_str());
				auto outputName = output["name"].get_string();
				std::string outputNameStr;
				outputNameStr.append(outputName.value());
				outputCreation.name = outputNameStr;

				switch (outputCreation.type)
				{
				case FrameGraphResourceType_Attachment:
				case FrameGraphResourceType_Texture:
				{
					std::string format = std::string(output["format"].get_string().value());

					outputCreation.resourceInfo.texture.format = VkFormatFromString(format.c_str());

					std::string loadOp = std::string(output["op"].get_string().value());

					outputCreation.resourceInfo.texture.loadOp = RenderPassOperationFromString(loadOp.c_str());

					auto resolution = output["resolution"].get_array();

					std::vector<uint32_t> values;
					for (auto r : resolution)
					{
						values.push_back((uint32_t)r.get_int64());
					}

					outputCreation.resourceInfo.texture.handle = { k_INVALID_HANDLE };
					outputCreation.resourceInfo.texture.width = values[0];
					outputCreation.resourceInfo.texture.height = values[1];
					outputCreation.resourceInfo.texture.depth = 1;

				}break;
				case FrameGraphResourceType_Buffer:
				{
					//TODO
				}
				break;


				default:
					break;
				}

				nodeCreation.outputs.push_back(outputCreation);
			}

			FrameGraphNodeHandle nodeHandle = builder->CreateNode(nodeCreation);

			nodes.push_back(nodeHandle);
		}

		LOG("FrameGraph Parse End");
	}
	void FrameGraph::Reset()
	{
		//Todo:: implement
	}
	void FrameGraph::AllocateRequiredResources()
	{
		LOG("FrameGraph: Allocating Resources");
		for (auto nodeHandle : nodes)
		{
			auto node = builder->GetNode(nodeHandle);

			for (auto resourceHandle : node->outputs)
			{
				auto r = builder->GetResource(resourceHandle);
				auto resource = builder->GetOutputResource(r->name.c_str());

				LOG("Allocating {}", r->name);

				if (resource->allocated)
				{
					continue;
				}

				switch (resource->type)
				{
				default:
					break;
				case FrameGraphResourceType_Invalid:
					break;
				case FrameGraphResourceType_Buffer:
					if (resource->resourceInfo.texture.handle.index == k_INVALID_HANDLE)
					{

					}
					break;
				case FrameGraphResourceType_Texture:
					break;
				case FrameGraphResourceType_Attachment:
					break;
				case FrameGraphResourceType_Reference:
					break;
				}

				if (resource->resourceInfo.texture.handle.index == k_INVALID_HANDLE)
				{
					auto& info = resource->resourceInfo.texture;
					TextureCreationData textureData;
					textureData.depth = info.depth;
					textureData.width = info.width;
					textureData.height = info.height;
					textureData.flags = (TextureFlags::Enum)TextureFlags::RenderTargetMask;
					textureData.format = info.format;
					textureData.name = resource->name.c_str();
					textureData.mipmaps = 1;
					auto handle = builder->graphics->CreateTexture(textureData);

					info.handle = handle;
				}

				resource->allocated = true;
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
		for (auto nodeHande : nodes) {
			FrameGraphNode* node = builder->GetNode(nodeHande);

			node->edges.clear();
		}

		for (auto nodeHandle : nodes) {
			FrameGraphNode* node = builder->GetNode(nodeHandle);

			if (!node->enabled)
			{
				continue;
			}

			ComputeEdges(this, node, nodeHandle);
		}

		//Sorted nodes in reverse order
		std::vector<FrameGraphNodeHandle> sortedNodes;
		sortedNodes.reserve(nodes.size());

		std::vector<uint8_t> visited;
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

				if (node->edges.size() == 0) {
					continue; //Leaf node, nothing to do here
				}

				//Add child handles
				for (auto edge : node->edges)
				{
					if (!visited[edge.index])
					{
						nodesToBeVisitedStack.push_back(edge);
					}
				}
			}
		}

		nodes.clear();

		for (int32_t i = sortedNodes.size() - 1; i >= 0; i--)
		{
			nodes.push_back(sortedNodes[i]);
		}

		//Allocate resource handles
		std::vector<FrameGraphNodeHandle> allocations;
		allocations.resize(builder->resourceCache.resources.Size(), { k_INVALID_HANDLE });

		for (auto nodeHandle : nodes)
		{
			FrameGraphNode* node = builder->GetNode(nodeHandle);

			if (!node->enabled)
			{
				continue;
			}

			for (auto outputNodeHandle : node->outputs)
			{
				uint32_t resourceIndex = outputNodeHandle.index;
				FrameGraphResource* resource = builder->GetResource(outputNodeHandle);

				if (!resource->resourceInfo.external && allocations[resourceIndex].index == k_INVALID_HANDLE)
				{
					allocations[resourceIndex] = nodeHandle;

					if (resource->type == FrameGraphResourceType_Attachment)
					{
						FrameGraphResourceInfo& info = resource->resourceInfo;

					}
				}
			}
		}

		for (auto nodeHandle : nodes)
		{
			auto node = builder->GetNode(nodeHandle);

			if (!node->enabled)
			{
				continue;
			}

			if (node->renderPass.index == k_INVALID_HANDLE)
			{
				CreateRenderPass(this, node);
			}
			if (node->framebuffer.index == k_INVALID_HANDLE)
			{
				CreateFrameBuffer(this, node);
			}
		}

		LOG("FrameGraph Compile End");
	}
	void FrameGraph::Render(RenderFrameData& renderData)
	{
		auto commands = renderData.currentCommand;

		for (auto nodeHandle : nodes)
		{
			FrameGraphNode* node = builder->GetNode(nodeHandle);
			//TODO: PUT MARKERS

			commands->Clear(vec4(0.2, 0.2, 0.3, 1.0));
			commands->ClearDepthStencil(1.0f, 0);

			uint16_t width = 0;
			uint16_t height = 0;

			for (auto nodeInputHandle : node->inputs)
			{
				FrameGraphResource* resource = builder->GetResource(nodeInputHandle);

				if (resource->type == FrameGraphResourceType_Texture)
				{
					auto texture = commands->graphics->Resources()->GetTexture(resource->resourceInfo.texture.handle);

					commands->AddImageBarrier(texture->vkImageHandle, RESOURCE_STATE_RENDER_TARGET, RESOURCE_STATE_PIXEL_SHADER_RESOURCE, 0, 1, resource->resourceInfo.texture.format == VK_FORMAT_D32_SFLOAT);
				}
				else if (resource->type == FrameGraphResourceType_Attachment)
				{
					auto texture = commands->graphics->Resources()->GetTexture(resource->resourceInfo.texture.handle);

					width = texture->width;
					height = texture->height;
				}
			}

			for (auto nodeOutputHandle : node->outputs)
			{
				FrameGraphResource* resource = builder->GetResource(nodeOutputHandle);

				if (resource->type == FrameGraphResourceType_Attachment)
				{
					auto texture = commands->graphics->Resources()->GetTexture(resource->resourceInfo.texture.handle);

					width = texture->width;
					height = texture->height;

					if (texture->format == VK_FORMAT_D32_SFLOAT)
					{
						commands->AddImageBarrier(texture->vkImageHandle, RESOURCE_STATE_UNDEFINED, RESOURCE_STATE_DEPTH_WRITE, 0, 1, resource->resourceInfo.texture.format == VK_FORMAT_D32_SFLOAT);
					}
					else
					{
						commands->AddImageBarrier(texture->vkImageHandle, RESOURCE_STATE_UNDEFINED, RESOURCE_STATE_RENDER_TARGET, 0, 1, resource->resourceInfo.texture.format == VK_FORMAT_D32_SFLOAT);
					}
				}
			}

			commands->SetScissor(0, 0, width, height);
			commands->SetViewport({ {0,0,width,height},0,1 });

			auto renderPass = renderData.m_renderPassesByType->find(node->type)->second;
			renderData.activeRenderTarget = renderData.graphics->Resources()->GetTexture(builder->GetResource(node->outputs[0])->resourceInfo.texture.handle);
			renderPass.PreRender(renderData);
			commands->BindRenderPass(node->renderPass, node->framebuffer);

			renderPass.Render(renderData);

			commands->EndCurrentRenderPass();
			//TODO: IMPLEMENT MARKERS
		}
	}
	void FrameGraph::OnResize(PuduGraphics& gpu, uint32_t new_width, uint32_t new_height)
	{
	}
	void FrameGraph::AttachRenderPass(FrameGraphRenderPass renderPass, RenderPassType type)
	{

	}

	FrameGraphNode* FrameGraph::GetNode(char* name)
	{
		return builder->GetNode(name);
	}
	FrameGraphNode* FrameGraph::GetNode(FrameGraphNodeHandle handle)
	{
		return builder->GetNode(handle);
	}
	FrameGraphResource* FrameGraph::GetOutputResource(char const* name)
	{
		return builder->GetOutputResource(name);
	}
	FrameGraphResource* FrameGraph::GetResource(FrameGraphResourceHandle handle)
	{
		return builder->GetResource(handle);
	}
	void FrameGraph::AddNode(FrameGraphNodeCreation& node)
	{
	}
	void FrameGraphRenderPassCache::Init()
	{
	}
	void FrameGraphRenderPassCache::Shutdown()
	{
	}
}
