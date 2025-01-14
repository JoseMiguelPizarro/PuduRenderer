
#include <vulkan/vulkan_core.h>
#include "ShaderCompiler.h"
#include "FileManager.h"
#include <iostream>
#include <boolinq.h>

using namespace boolinq;


void PrintVariable(VariableReflection* variable) {
	printf("%s", variable->getName());

}

void PrintType(TypeReflection* type) {
	printf(type->getName());

}

namespace Pudu {

	void ShaderCompilation::AddKernel(const char* name, ShaderKernel& kernel) {
		m_kernelsByName[name] = kernel;
	}

	void ShaderCompiler::Init()
	{
		createGlobalSession(m_globalSession.writeRef());

		TargetDesc targetDesc;
		targetDesc.profile = m_globalSession->findProfile("spirv_1_5");
		targetDesc.format = SLANG_SPIRV;

		const char* searchPaths[] = { "Assets/Shaders" };

		SessionDesc sessionDesc;
		sessionDesc.defaultMatrixLayoutMode = SLANG_MATRIX_LAYOUT_COLUMN_MAJOR;
		sessionDesc.targetCount = 1;
		sessionDesc.targets = &targetDesc;
		sessionDesc.searchPathCount = 1;
		sessionDesc.searchPaths = searchPaths;

		slang::CompilerOptionEntry useEntryPointNameOption;
		useEntryPointNameOption.name = slang::CompilerOptionName::VulkanUseEntryPointName;
		useEntryPointNameOption.value = { slang::CompilerOptionValueKind::Int,1, 0, nullptr, nullptr };

		std::array<slang::CompilerOptionEntry, 1> options =
		{
			useEntryPointNameOption
		};

		sessionDesc.compilerOptionEntries = options.data();
		sessionDesc.compilerOptionEntryCount = options.size();


		m_globalSession->createSession(sessionDesc, m_session.writeRef());
	}

	ShaderCompilation ShaderCompiler::Compile(const char* path, std::vector<const char*> entryPoints, bool compute)
	{
		Slang::ComPtr<IBlob> diagnostics;
		IModule* baseModule = m_session->loadModule("PuduGraphics.slang");
		IModule* module = m_session->loadModule(path, diagnostics.writeRef());

		std::vector<Slang::ComPtr<IEntryPoint>> slangEntryPoints;

		for (auto entryPoint : entryPoints) {
			Slang::ComPtr<IEntryPoint> e;

			module->findEntryPointByName(entryPoint, e.writeRef());

			slangEntryPoints.push_back(e);
		}

		std::vector<IComponentType*> components = {};
		if (!compute)
		{
			components.push_back(baseModule);
		}

		components.push_back(module);

		components.append_range(slangEntryPoints);

		Slang::ComPtr<IComponentType> program;
		m_session->createCompositeComponentType(components.data(), components.size(), program.writeRef(), diagnostics.writeRef());

		if (diagnostics)
		{
			fprintf(stderr, "%s\n", (const char*)diagnostics->getBufferPointer());
		}

		slang::ProgramLayout* layout = program->getLayout();

		Slang::ComPtr<IComponentType> linkedProgram;
		program->link(linkedProgram.writeRef(), diagnostics.writeRef());

		if (diagnostics)
		{
			fprintf(stderr, "%s\n", (const char*)diagnostics->getBufferPointer());
		}

		//Global
		printf("Global scope \n");
		auto globalTypeLayout = layout->getGlobalParamsTypeLayout();

		ShaderCompilation compiledData;

		ShaderObjectLayoutBuilder layoutBuilder;
		layoutBuilder.m_globalSession = m_globalSession;
		layoutBuilder.addBindingsForParameterBlock(globalTypeLayout, compiledData.descriptorsData);

		/*Slang::ComPtr<IBlob> spirvCode;
		{
			linkedProgram->getTargetCode(0, spirvCode.writeRef(), diagnostics.writeRef());
		}*/

		if (diagnostics)
		{
			fprintf(stderr, "%s\n", (const char*)diagnostics->getBufferPointer());
		}

		for (size_t i = 0; i < entryPoints.size(); i++)
		{
			Slang::ComPtr<IBlob> kernel;
			linkedProgram->getEntryPointCode(i, 0, kernel.writeRef(), diagnostics.writeRef());

			ShaderKernel kernelData;
			auto p = kernel->getBufferPointer();
			kernelData.code = static_cast<const uint32_t*>(kernel->getBufferPointer());
			kernelData.codeSize = kernel->getBufferSize();

			compiledData.AddKernel(entryPoints[i], kernelData);
		}

		if (diagnostics)
		{
			fprintf(stderr, "%s\n", (const char*)diagnostics->getBufferPointer());
		}

		return compiledData;
	}

	VkDescriptorType ToVk(slang::BindingType slangBindingType)
	{
		switch (slangBindingType)
		{
		case slang::BindingType::PushConstant:
		default:
			std::fprintf(stderr, "Assertion failed: %s %d\n", "Unsupported binding type!", slangBindingType);
			//std::abort();
			return VK_DESCRIPTOR_TYPE_MAX_ENUM;

		case slang::BindingType::Sampler:
			return VK_DESCRIPTOR_TYPE_SAMPLER;
		case slang::BindingType::CombinedTextureSampler:
			return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		case slang::BindingType::Texture:
			return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		case slang::BindingType::MutableTexture:
			return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		case slang::BindingType::TypedBuffer:
			return VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
		case slang::BindingType::MutableTypedBuffer:
			return VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
		case slang::BindingType::RawBuffer:
		case slang::BindingType::MutableRawBuffer:
			return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		case slang::BindingType::InputRenderTarget:
			return VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		case slang::BindingType::InlineUniformData:
			return VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT;
		case slang::BindingType::RayTracingAccelerationStructure:
			return VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
		case slang::BindingType::ConstantBuffer:
			return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		}
	}

	void Tab(uint32_t count) {
		for (size_t i = 0; i < count; i++)
		{
			printf("%s", "\t");
		}
	}

	void ShaderObjectLayoutBuilder::addBindingsForParameterBlock(slang::TypeLayoutReflection* typeLayout, DescriptorSetLayoutsData& layoutsData)
	{
		auto fieldCount = typeLayout->getFieldCount();
		uint16_t setsCount = 0;
		for (size_t i = 0; i < fieldCount; i++)
		{
			auto field = typeLayout->getFieldByIndex(i);

			if (field->getType()->getKind() == TypeReflection::Kind::ConstantBuffer)
			{
				//We'll only process parameterblocks
				continue;
			}

			auto fieldLayout = field->getTypeLayout()->getElementTypeLayout();

			size_t descriptorSet = field->getOffset(SLANG_PARAMETER_CATEGORY_SUB_ELEMENT_REGISTER_SPACE);

			auto propertiesCount = fieldLayout->getFieldCount();

			std::vector<DescriptorSetLayoutData*> layoutsPtr;

			for (size_t i = 0; i < layoutsData.layoutData.size(); i++)
			{
				layoutsPtr.push_back(&layoutsData.layoutData[i]);
			}

			DescriptorSetLayoutData layoutData{};


			bool layoutExists = from(layoutsPtr).any([descriptorSet](const DescriptorSetLayoutData* l) { return l->SetNumber == descriptorSet; });

			auto bindless = field->getVariable()->findUserAttributeByName(m_globalSession, "Bindless");
			bool isBindless = false;

			if (bindless)
			{
				int v;
				bindless->getArgumentValueInt(0, &v);
				isBindless = bindless;
			}

			for (size_t j = 0; j < propertiesCount; j++)
			{
				auto property = fieldLayout->getFieldByIndex(j);

				uint16_t bindingCount = 1;

				if (property->getType()->getKind() == TypeReflection::Kind::Array)
				{
					bindingCount = property->getTypeLayout()->getElementCount();
				}

				DescriptorBinding binding;

				auto descriptorType = ToVk(property->getTypeLayout()->getBindingRangeType(0));
				binding.set = static_cast<uint16_t>(descriptorSet);
				binding.index = property->getBindingIndex();
				binding.type = descriptorType;
				binding.count = bindingCount;
				binding.name = std::format("{}.{}", field->getName(), property->getName());

				layoutsData.bindingsData.push_back(binding);

				VkDescriptorSetLayoutBinding layoutBinding = {};
				layoutBinding.binding = binding.index;
				layoutBinding.descriptorType = descriptorType;
				layoutBinding.descriptorCount = bindingCount;
				layoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_COMPUTE_BIT; //TODO: Hack, but in all stages for now
				layoutBinding.pImmutableSamplers = nullptr;
				layoutData.Bindings.push_back(layoutBinding);
			}
			if (!layoutExists)
			{
				layoutData.SetNumber = descriptorSet;
				layoutData.CreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
				layoutData.CreateInfo.bindingCount = layoutData.Bindings.size(); //We might have issues here if the binding count doesn't consider the ones in vertex/fragment
				layoutData.CreateInfo.pBindings = layoutData.Bindings.data();
				layoutData.name = field->getName();
				layoutData.bindless = isBindless;
				layoutsData.layoutData.push_back(layoutData);

				setsCount++;
			}
		}

		layoutsData.setsCount = setsCount;
	}
}



