#pragma once
#include <vulkan/vulkan_core.h>

namespace Pudu
{
	enum LoadOperation
	{
		DontCare,
		Load,
		Clear,
	}; // enum Enum

	enum AttachmentUsage {
		Read = 1,
		Write = 2,
		ReadAndWrite = 3,
		Sample = 4
	};

	enum CullMode {
		Front,
		Back,
		Both,
		None
	};


	static VkCullModeFlagBits ToVk(CullMode cullMode) {
		switch (cullMode)
		{
		case CullMode::Back:
			return VK_CULL_MODE_BACK_BIT;
			break;
		case CullMode::Front:
			return VK_CULL_MODE_FRONT_BIT;
		case CullMode::Both:
			return VK_CULL_MODE_FRONT_AND_BACK;
		case CullMode::None:
			return VK_CULL_MODE_NONE;
		default:
			return VK_CULL_MODE_BACK_BIT;
			break;
		}
	}

	static VkAttachmentLoadOp ToVk(LoadOperation op) {
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
		default:
			break;
		}

		return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	}

	static VkAttachmentStoreOp ToVk(AttachmentUsage usage) {
		switch (usage)
		{
		case Pudu::ReadAndWrite:
			return VK_ATTACHMENT_STORE_OP_STORE;
			break;
		case Pudu::Write:
			return VK_ATTACHMENT_STORE_OP_STORE;
			break;
		case Pudu::Read:
			return VK_ATTACHMENT_STORE_OP_DONT_CARE;
			break;
		case Pudu::Sample:
			return VK_ATTACHMENT_STORE_OP_DONT_CARE;
			break;
		default:
			break;
		}

		return VK_ATTACHMENT_STORE_OP_DONT_CARE;
	}
}