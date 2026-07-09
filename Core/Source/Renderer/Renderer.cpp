#include "Renderer.h"
#include "Ping/Types.h"

#include "ECS/Components/Texture.h"
#include "ECS/Components/Transform.h"

#include <chrono>
#include <glm/gtc/matrix_transform.hpp>

using namespace Mupfel;

static const std::vector<Transform> vertices = {
	{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
	{{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
	{{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
	{{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}};

static const std::vector<uint16_t> indices = {0, 1, 2, 2, 3, 0};

void Mupfel::Renderer::Init(const Ping::Device& device, const Window& window)
{
	logger = Logger::Create("Renderer");
	logger->info("Init");
	swapchain = device.CreateSwapChain(window, frames_in_flight);
	Ping::PipelineSpecification pipeline_spec{
		"Shaders/slang.spv",
		Transform::GetVertexLayout(),
		{{.binding = 0, .type = Ping::DescriptorType::UniformBuffer, .stageFlags = Ping::ShaderStage::Vertex}}};
	pipeline = device.CreatePipeline(pipeline_spec, swapchain.value());
	commandBuffers = device.CreateCommandBuffers(Ping::QueueType::Graphics, frames_in_flight);
	assert(commandBuffers.has_value() && commandBuffers.value().size() > 0);

	/* We need one vertex buffer for each frame in flight */
	for (uint32_t i = 0; i < frames_in_flight; i++)
	{
		auto& buffer = vertex_buffers.emplace_back(device.CreateBuffer(
			sizeof(Transform) * vertices.size(), Ping::BufferUsage::VertexBuffer,
			Ping::MemoryProperty::HostVisible | Ping::MemoryProperty::HostCoherent));
		auto* mapped_ptr = static_cast<Transform*>(buffer.GetMappedPtr());

		/* Copy vertices */
		std::memcpy(mapped_ptr, vertices.data(), buffer.Size());

		uniformBuffers.emplace_back(device.CreateBuffer(
			sizeof(UniformBufferObject), Ping::BufferUsage::UniformBuffer,
			Ping::MemoryProperty::HostVisible | Ping::MemoryProperty::HostCoherent));
	}

	index_buffer = std::move(device.CreateBuffer(
		sizeof(uint16_t) * indices.size(), Ping::BufferUsage::IndexBuffer | Ping::BufferUsage::TransferDst,
		Ping::MemoryProperty::DeviceLocal));

	/* Copy indices */
	index_buffer.value().CopyHostData(device, indices.data(), sizeof(uint16_t) * indices.size());

	/* Create descriptor sets */
	descriptorSets = device.CreateDescriptorSets(pipeline.value(), uniformBuffers);
}

void Mupfel::Renderer::SyncRenderableObjects(World& world, const Ping::Device& device, uint32_t frame_index)
{ /* We are currently not taking any data from the CPU */ }

void Mupfel::Renderer::updateMVP(Ping::Buffer& uniform_buffer, Ping::SwapChain& swapchain)
{
	auto [width, height] = swapchain.GetExtent();

	static auto startTime = std::chrono::high_resolution_clock::now();

	auto  currentTime = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

	UniformBufferObject ubo{};
	ubo.model = rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	ubo.view = lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	ubo.proj =
		glm::perspective(glm::radians(45.0f), static_cast<float>(width) / static_cast<float>(height), 0.1f, 10.0f);
	ubo.proj[1][1] *= -1;
	std::memcpy(uniform_buffer.GetMappedPtr(), &ubo, sizeof(UniformBufferObject));
}

void Mupfel::Renderer::RenderNextFrame(World& world, const Ping::Device& device, const Window& window)
{

	Ping::CommandBuffer& current_command_buffer = commandBuffers.value()[frameIndex];

	current_command_buffer.WaitForFences(device);

	updateMVP(uniformBuffers[frameIndex], swapchain.value());

	SyncRenderableObjects(world, device, frameIndex);

	uint32_t image_index = swapchain.value().AcquireNextImage(frameIndex);

	/* Check if the index is valid */
	if (image_index == std::numeric_limits<uint32_t>::max())
	{
		/* Image was resized, swapchain needs to be recreated */
		swapchain.value().Recreate(device, window, frames_in_flight);
		return;
	}

	current_command_buffer.Begin(device, Ping::CommandBufferUsage::None);

	Ping::ImageLayoutTransition layout_transition = {
		.oldLayout = Ping::ImageLayout::Undefined,
		.newLayout = Ping::ImageLayout::ColorAttachmentOptimal,
		.srcAccessMask = Ping::AccessMask::None,
		.dstAccessMask = Ping::AccessMask::ColorAttachmentWrite,
		.srcStage = Ping::PipelineStage::ColorAttachmentOutput,
		.dstStage = Ping::PipelineStage::ColorAttachmentOutput};

	current_command_buffer.transitionImageLayout(swapchain.value(), image_index, layout_transition);

	current_command_buffer.BeginRendering(swapchain.value(), image_index);

	current_command_buffer.BindPipeline(pipeline.value());

	current_command_buffer.BindDescriptorSet(pipeline.value(), descriptorSets.value(), frameIndex);

	current_command_buffer.BindVertexBuffer(pipeline.value(), vertex_buffers[frameIndex], 0);

	current_command_buffer.BindIndexBuffer(pipeline.value(), index_buffer.value());

	// current_command_buffer.Draw(3);
	current_command_buffer.DrawIndexed(indices.size());

	current_command_buffer.EndRendering();

	layout_transition.oldLayout = Ping::ImageLayout::ColorAttachmentOptimal;
	layout_transition.newLayout = Ping::ImageLayout::PresentSource;
	layout_transition.srcAccessMask = Ping::AccessMask::ColorAttachmentWrite;
	layout_transition.dstAccessMask = Ping::AccessMask::None;
	layout_transition.dstStage = Ping::PipelineStage::BottomOfPipe;

	current_command_buffer.transitionImageLayout(swapchain.value(), image_index, layout_transition);

	current_command_buffer.End();

	current_command_buffer.Submit(device, swapchain.value(), frameIndex, image_index);

	if (!swapchain.value().Present(device, image_index))
	{
		/* Image was resized, swapchain needs to be recreated */
		swapchain.value().Recreate(device, window, frames_in_flight);
	}

	incrementFrameIndex();
}

void Mupfel::Renderer::Shutdown() {}

void Mupfel::Renderer::incrementFrameIndex() { frameIndex = (frameIndex + 1) % frames_in_flight; }
