#pragma once
#include <memory>
#include <string>
#include <vector>
#include <vulkan/vulkan_core.h>

class ShaderModule;
class IShaderDescriptor;

class DescriptorBatch final
{
public:
	template<typename UniformClass, typename... Args>
	void add_descriptor(Args&&... args)
	{
		bindings.push_back(std::make_shared<UniformClass>(std::forward<Args>(args)...));
	}

	std::vector<VkDescriptorSetLayoutBinding> build(const std::shared_ptr<ShaderModule>& vertex_module, const std::shared_ptr<ShaderModule>& fragment_module, const std::shared_ptr<ShaderModule>& geometry_module);

private:
	std::vector<std::shared_ptr<IShaderDescriptor>> bindings;
};


class IShaderDescriptor
{
public:
	IShaderDescriptor(const std::string& in_binding_name, VkShaderStageFlags in_shader_stage)
	: binding_name(in_binding_name), shader_stage(in_shader_stage) {}
	virtual ~IShaderDescriptor() = default;

	VkDescriptorSetLayoutBinding build(const std::shared_ptr<ShaderModule>& vertex_module, const std::shared_ptr<ShaderModule>& fragment_module, const std::shared_ptr<ShaderModule>& geometry_module);

protected:

	[[nodiscard]] virtual VkDescriptorSetLayoutBinding get() const = 0;

private:
	
	const std::string binding_name;
	const VkShaderStageFlags shader_stage;	
};

class TextureDescriptor final : public IShaderDescriptor
{
public:
	using IShaderDescriptor::IShaderDescriptor;
protected:
	[[nodiscard]] VkDescriptorSetLayoutBinding get() const override {
		VkDescriptorSetLayoutBinding samplerLayoutBinding{};
		samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplerLayoutBinding.descriptorCount = 1;
		samplerLayoutBinding.pImmutableSamplers = nullptr;
		return samplerLayoutBinding;
	}
};