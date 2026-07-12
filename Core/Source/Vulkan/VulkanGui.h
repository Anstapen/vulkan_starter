#pragma once
#include "VulkanBuffer.h"
#include "VulkanCommon.h"
#include "VulkanImage.h"
#include "VulkanSampler.h"
#include <vector>

struct ImGuiContext;

namespace Backend
{
class VKManager;

class VulkanGui
{
	friend class VKManager;

public:
	VulkanGui(
		vk::raii::DescriptorPool&&		descriptor_pool,
		vk::raii::DescriptorSetLayout&& descriptor_set_layout,
		vk::raii::PipelineLayout&&		pipeline_layout,
		vk::raii::Pipeline&&			pipeline,
		VulkanImage&&					font_image,
		VulkanSampler&&					font_sampler,
		vk::raii::DescriptorSet&&		font_descriptor_set,
		std::vector<VulkanBuffer>&&		vertex_buffers,
		std::vector<VulkanBuffer>&&		index_buffers,
		ImGuiContext*					imgui_context) noexcept;
	~VulkanGui();
	VulkanGui(const VulkanGui& other) = delete;
	VulkanGui(VulkanGui&& other) noexcept;
	VulkanGui& operator=(const VulkanGui& other) = delete;
	VulkanGui& operator=(VulkanGui&& other) noexcept;

	void NewFrame();

private:
	vk::raii::DescriptorPool	  descriptorPool;
	vk::raii::DescriptorSetLayout descriptorSetLayout;
	vk::raii::PipelineLayout	  pipelineLayout;
	vk::raii::Pipeline			  pipeline;
	VulkanImage					  fontImage;
	VulkanSampler				  fontSampler;
	vk::raii::DescriptorSet		  fontDescriptorSet;
	std::vector<VulkanBuffer>	  vertexBuffers;
	std::vector<VulkanBuffer>	  indexBuffers;
	ImGuiContext*				  imguiContext;
};

} // namespace Backend
