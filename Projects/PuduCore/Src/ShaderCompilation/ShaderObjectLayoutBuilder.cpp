//
// Created by Hojaverde on 2/23/2025.
//

#include "ShaderCompilation/ShaderObjectLayoutBuilder.h"

#include <boolinq.h>

#include "Logger.h"

namespace Pudu {

	CumulativeOffset ShaderObjectLayoutBuilder::CalculateCumulativeOffset(
		slang::VariableLayoutReflection* variableLayout, slang::ParameterCategory layoutUnit, AccessPath accessPath)
	{
	}

	void ShaderObjectLayoutBuilder::AddBindingsForParameterBlock(slang::TypeLayoutReflection* typeLayout, DescriptorSetLayoutsData& layoutsData)
	{
		auto fieldCount = typeLayout->getFieldCount();
		u16 setsCount = 0;


		for (size i = 0; i < fieldCount; i++)
		{
			auto field = typeLayout->getFieldByIndex(i);
			auto fieldLayout = field->getTypeLayout()->getElementTypeLayout();

			auto containerLayout = field->getTypeLayout()->getContainerVarLayout();


			LOG("Shader field: {} category: {}", field->getName(), (u32)fieldLayout->getParameterCategory());

			size descriptorSet = field->getOffset(SLANG_PARAMETER_CATEGORY_SUB_ELEMENT_REGISTER_SPACE);

			slang::BindingType type = fieldLayout->getBindingRangeType(0);
			if (type == slang::BindingType::PushConstant)
			{
				LOG("Push constants! {} set {} binding {}", field->getName(), descriptorSet, 0);
			}

			if (field->getType()->getKind() == TypeReflection::Kind::ConstantBuffer)
			{
				LOG("Constant buffer layout not supported {}", field->getName());
				//We'll only process parameterblocks
				continue;
			}

			auto propertiesCount = fieldLayout->getFieldCount();

			std::vector<DescriptorSetLayoutData*> layoutsPtr;

			for (size_t i = 0; i < layoutsData.layoutData.size(); i++)
			{
				layoutsPtr.push_back(&layoutsData.layoutData[i]);
			}

			DescriptorSetLayoutData layoutData{};


			bool layoutExists = boolinq::from(layoutsPtr).any([descriptorSet](const DescriptorSetLayoutData* l) { return l->SetNumber == descriptorSet; });

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

				LOG("Shader property: {}", property->getName());


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

	void ShaderObjectLayoutBuilder::ParseShaderProgramLayout(slang::ProgramLayout* programLayout,
		ShaderCompilationObject& outCompilationObject)
	{
		auto globalVarLayout = programLayout->getGlobalParamsVarLayout();

		AccessPath rootOffsets;
		rootOffsets.valid = true;

		ShaderLayoutBuilderContext context;
		context.accessPath = &rootOffsets;
		context.shaderCompilationObject = &outCompilationObject;

		ParseScope(globalVarLayout, context);
	}

	void ShaderObjectLayoutBuilder::ParseVariableLayout(VariableLayoutReflection* varLayout,
		ShaderLayoutBuilderContext& context)
	{

	}

	void ShaderObjectLayoutBuilder::ParseVariableOffsets(VariableLayoutReflection* varLayout,
		ShaderLayoutBuilderContext& context)
	{
	}

	void ShaderObjectLayoutBuilder::ParseScope(slang::VariableLayoutReflection* scopeVarLayout, ShaderLayoutBuilderContext& context)
	{
		ExtendedAccessPath(*context.accessPath, scopeVarLayout);


		TypeLayoutReflection* scopeTypeLayout = scopeVarLayout->getTypeLayout();

		switch (scopeTypeLayout->getKind())
		{
		case TypeReflection::Kind::Struct:
			u16 paramCount = scopeTypeLayout->getFieldCount();
			for (u16 i = 0; i < paramCount; i++)
			{
				auto param = scopeTypeLayout->getFieldByIndex(i);

				ParseVariableLayout(param, context);
			}


			break;

		case TypeReflection::Kind::ConstantBuffer:
			//Constant buffer detected.
			auto containerLayout = scopeTypeLayout->getContainerVarLayout();
			ParseVariableOffsets(containerLayout, context);
			ParseScope(scopeTypeLayout->getElementVarLayout(), context);
			break;

		case TypeReflection::Kind::ParameterBlock:
			ParseVariableOffsets(scopeTypeLayout->getElementVarLayout(), context);
			ParseScope(scopeTypeLayout->getElementVarLayout(), context);

		default:
			break;
		}
	}
} // Pudu