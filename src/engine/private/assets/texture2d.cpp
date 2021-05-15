
#include "assets/texture2d.h"

#include "rendering/window.h"

#include <cmath>
#include <stb_image.h>



#include "jobSystem/job.h"
#include "jobSystem/job_system.h"
#include "rendering/vulkan/descriptorPool.h"
#include "rendering/vulkan/texture.h"
#include "ui/window/windows/profiler.h"

Texture2d::Texture2d(const std::filesystem::path& texture_path)
{
	if (!std::filesystem::exists(texture_path))
	{
            LOG_ERROR("cannot find texture asset %s", texture_path.string().c_str());
		return;
	}
	
	texture_data = stbi_load(texture_path.string().c_str(), &texture_width, &texture_height, &texture_channels, 4);
	texture_channels = 4;
	load_image();
}

Texture2d::Texture2d(uint8_t* data, size_t width,
                     size_t height, uint8_t channel_count)
	: texture_data(data), texture_width(static_cast<int>(width)), texture_height(static_cast<int>(height)), texture_channels(4)
{
	load_image();
}

Texture2d::~Texture2d()
{
	creation_job->wait();

	if (descriptor_image_info) delete descriptor_image_info;
	if (image_sampler != VK_NULL_HANDLE) vkDestroySampler(window_context->get_context()->logical_device, image_sampler, vulkan_common::allocation_callback);
	if (image_view != VK_NULL_HANDLE) vkDestroyImageView(window_context->get_context()->logical_device, image_view, vulkan_common::allocation_callback);

	if (image != VK_NULL_HANDLE) vkDestroyImage(window_context->get_context()->logical_device, image, vulkan_common::allocation_callback);
	if (image_memory != VK_NULL_HANDLE) vkFreeMemory(window_context->get_context()->logical_device, image_memory, vulkan_common::allocation_callback);

	if (image_descriptor_layout != VK_NULL_HANDLE) vkDestroyDescriptorSetLayout(window_context->get_context()->logical_device, image_descriptor_layout, vulkan_common::allocation_callback);

	std::free(texture_data);
}

bool Texture2d::try_load()
{
	return creation_job && creation_job->is_complete();
}

VkDescriptorImageInfo* Texture2d::get_descriptor_info()
{
	if (!descriptor_image_info)
	{
		descriptor_image_info = new VkDescriptorImageInfo();
		descriptor_image_info->sampler = image_sampler;
		descriptor_image_info->imageView = image_view;
		descriptor_image_info->imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	}
	return descriptor_image_info;
}

void Texture2d::load_image()
{
	creation_job = job_system::new_job([&] {
		BEGIN_NAMED_RECORD(LOAD_TEXTURE_2D);
		create_image();
		create_image_sampler();
		create_image_descriptors();
        LOG_INFO("created texture 2d %s", asset_id->to_string().c_str());
		});
}

void Texture2d::create_image()
{
	VkFormat imageFormat = VK_FORMAT_UNDEFINED;
	switch (texture_channels) {
	case 1:
		imageFormat = VK_FORMAT_R8_SRGB;
		break;
	case 2:
		imageFormat = VK_FORMAT_R8G8_SRGB;
		break;
	case 3:
		imageFormat = VK_FORMAT_R8G8B8_SRGB;
		break;
	case 4:
		imageFormat = VK_FORMAT_R8G8B8A8_SRGB;
		break;
	}

	VkDeviceSize image_buffer_size = texture_width * texture_height * texture_channels;

	mip_level = static_cast<uint32_t>(std::floor(log2(std::max(texture_width, texture_height)))) + 1;

	VkBuffer staging_buffer;
	VkDeviceMemory stagingBufferMemory;
	vulkan_utils::create_buffer(window_context->get_context(), image_buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staging_buffer, stagingBufferMemory);

	// Copy memory
	void* memory_data;
	vkMapMemory(window_context->get_context()->logical_device, stagingBufferMemory, 0, image_buffer_size, 0, &memory_data);
	memcpy(memory_data, texture_data, static_cast<size_t>(image_buffer_size));
	vkUnmapMemory(window_context->get_context()->logical_device, stagingBufferMemory);

	vulkan_texture::create_image_2d(window_context->get_context(), static_cast<uint32_t>(texture_width), static_cast<uint32_t>(texture_height), mip_level,
		VK_SAMPLE_COUNT_1_BIT, imageFormat, VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
		VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image,
		image_memory);

	VkCommandBuffer command_buffer = vulkan_utils::begin_single_time_commands(window_context);

	vulkan_texture::transition_image_layout(image, imageFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mip_level, command_buffer);
	vulkan_texture::copy_buffer_to_image(staging_buffer, image, static_cast<uint32_t>(texture_width), static_cast<uint32_t>(texture_height), command_buffer);

	vulkan_texture::generate_mipmaps(window_context, image, imageFormat, static_cast<uint32_t>(texture_width), static_cast<uint32_t>(texture_height), mip_level, command_buffer);
	vulkan_utils::end_single_time_commands(window_context, command_buffer);

	vulkan_texture::create_image_view_2d(window_context->get_context(), image, image_view, imageFormat, VK_IMAGE_ASPECT_COLOR_BIT, mip_level);

	vkDestroyBuffer(window_context->get_context()->logical_device, staging_buffer, vulkan_common::allocation_callback);
	vkFreeMemory(window_context->get_context()->logical_device, stagingBufferMemory, vulkan_common::allocation_callback);
}

void Texture2d::create_image_sampler()
{
	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = 16.0f;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = static_cast<float>(mip_level);
	samplerInfo.mipLodBias = 0.0f; // Optional

	VK_ENSURE(vkCreateSampler(window_context->get_context()->logical_device, &samplerInfo, vulkan_common::allocation_callback, &image_sampler), "failed to create sampler");
}

void Texture2d::create_image_descriptors()
{
	VkDescriptorSetLayoutBinding binding[1] = {};
	binding[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	binding[0].descriptorCount = 1;
	binding[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	VkDescriptorSetLayoutCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	info.bindingCount = 1;
	info.pBindings = binding;
	vkCreateDescriptorSetLayout(window_context->get_context()->logical_device, &info, nullptr, &image_descriptor_layout);

	image_descriptors.resize(window_context->get_image_count());
	for (size_t i = 0; i < window_context->get_image_count(); ++i)
	{
		// Create Descriptor Set:
		VkDescriptorSetAllocateInfo alloc_info = {};
		alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		alloc_info.descriptorSetCount = 1;
		alloc_info.pSetLayouts = &image_descriptor_layout;
		window_context->get_descriptor_pool()->alloc_memory(alloc_info);
		vkAllocateDescriptorSets(window_context->get_context()->logical_device, &alloc_info, &image_descriptors[i]);

		// Update the Descriptor Set:
		VkDescriptorImageInfo desc_image[1] = {};
		desc_image[0].sampler = image_sampler;
		desc_image[0].imageView = image_view;
		desc_image[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		VkWriteDescriptorSet write_desc[1] = {};
		write_desc[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write_desc[0].dstSet = image_descriptors[i];
		write_desc[0].descriptorCount = 1;
		write_desc[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		write_desc[0].pImageInfo = desc_image;
		vkUpdateDescriptorSets(window_context->get_context()->logical_device, 1, write_desc, 0, NULL);
	}
}