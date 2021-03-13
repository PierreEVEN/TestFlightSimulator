
#include "rendering/uniforms/uniformBase.h"


#include "ios/logger.h"
#include "rendering/vulkan/shaderModule.h"

std::vector<VkDescriptorSetLayoutBinding> DescriptorBatch::build(const std::shared_ptr<ShaderModule>& vertex_module,
                                                              const std::shared_ptr<ShaderModule>& fragment_module, const std::shared_ptr<ShaderModule>& geometry_module)
{
	std::vector<VkDescriptorSetLayoutBinding> result;

	for (auto& binding : bindings)
	{
		result.push_back(binding->build(vertex_module, fragment_module, geometry_module));
	}
	return result;
}

VkDescriptorSetLayoutBinding IShaderDescriptor::build(const std::shared_ptr<ShaderModule>& vertex_module,
	const std::shared_ptr<ShaderModule>& fragment_module, const std::shared_ptr<ShaderModule>& geometry_module)
{
	VkDescriptorSetLayoutBinding result = get();
	result.stageFlags = shader_stage;
	std::shared_ptr<ShaderModule> stage_module;
	switch (shader_stage)
	{
	case VK_SHADER_STAGE_VERTEX_BIT:
		stage_module = vertex_module;
		break;
	case VK_SHADER_STAGE_FRAGMENT_BIT:
		stage_module = fragment_module;
		break;
	case VK_SHADER_STAGE_GEOMETRY_BIT:
		stage_module = geometry_module;
		break;
	default:
		logger_fail("wrong shader stage id");
	}
	
	return result;
}
