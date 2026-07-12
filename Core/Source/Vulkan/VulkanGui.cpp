#include "VulkanGui.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"

using namespace Backend;

VulkanGui::VulkanGui(
	vk::raii::DescriptorPool&&		descriptor_pool,
	vk::raii::DescriptorSetLayout&& descriptor_set_layout,
	vk::raii::PipelineLayout&&		pipeline_layout,
	vk::raii::Pipeline&&			pipeline,
	VulkanImage&&					font_image,
	VulkanSampler&&					font_sampler,
	vk::raii::DescriptorSet&&		font_descriptor_set,
	std::vector<VulkanBuffer>&&		vertex_buffers,
	std::vector<VulkanBuffer>&&		index_buffers,
	ImGuiContext*					imgui_context) noexcept
	: descriptorPool(std::move(descriptor_pool)), descriptorSetLayout(std::move(descriptor_set_layout)),
	  pipelineLayout(std::move(pipeline_layout)), pipeline(std::move(pipeline)), fontImage(std::move(font_image)),
	  fontSampler(std::move(font_sampler)), fontDescriptorSet(std::move(font_descriptor_set)),
	  vertexBuffers(std::move(vertex_buffers)), indexBuffers(std::move(index_buffers)), imguiContext(imgui_context)
{
}

VulkanGui::~VulkanGui()
{
	if (imguiContext)
	{
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext(imguiContext);
	}
}

VulkanGui::VulkanGui(VulkanGui&& other) noexcept
	: descriptorPool(std::move(other.descriptorPool)), descriptorSetLayout(std::move(other.descriptorSetLayout)),
	  pipelineLayout(std::move(other.pipelineLayout)), pipeline(std::move(other.pipeline)),
	  fontImage(std::move(other.fontImage)), fontSampler(std::move(other.fontSampler)),
	  fontDescriptorSet(std::move(other.fontDescriptorSet)), vertexBuffers(std::move(other.vertexBuffers)),
	  indexBuffers(std::move(other.indexBuffers)), imguiContext(other.imguiContext)
{
	other.imguiContext = nullptr;
}

VulkanGui& VulkanGui::operator=(VulkanGui&& other) noexcept
{
	descriptorPool = std::move(other.descriptorPool);
	descriptorSetLayout = std::move(other.descriptorSetLayout);
	pipelineLayout = std::move(other.pipelineLayout);
	pipeline = std::move(other.pipeline);
	fontImage = std::move(other.fontImage);
	fontSampler = std::move(other.fontSampler);
	fontDescriptorSet = std::move(other.fontDescriptorSet);
	vertexBuffers = std::move(other.vertexBuffers);
	indexBuffers = std::move(other.indexBuffers);
	imguiContext = other.imguiContext;
	other.imguiContext = nullptr;
	return *this;
}

void VulkanGui::NewFrame()
{
	ImGui::SetCurrentContext(imguiContext);
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}
