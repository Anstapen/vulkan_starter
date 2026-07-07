#include "Renderer.h"
#include "Ping/Types.h"

#include "ECS/Components/Texture.h"
#include "ECS/Components/Transform.h"

using namespace Mupfel;

void Mupfel::Renderer::Init(const Ping::Device& device, const Window& window)
{
	logger = Logger::Create("Renderer");
	logger->info("Init");
	swapchain = device.CreateSwapChain(window, frames_in_flight);
	Ping::PipelineSpecification pipeline_spec{"Shaders/slang.spv", Transform::GetVertexLayout()};
	pipeline = device.CreatePipeline(pipeline_spec, swapchain.value());
	commandBuffers = device.CreateCommandBuffers(Ping::QueueType::Graphics, frames_in_flight);
	assert(commandBuffers.has_value() && commandBuffers.value().size() > 0);

	/* We need one vertex buffer for each frame in flight */
	for (uint32_t i = 0; i < frames_in_flight; i++)
	{
		vertex_buffers.emplace_back(std::move(device.CreateBuffer(
			sizeof(Transform) * 100, Ping::BufferUsage::VertexBuffer, Ping::MemoryProperty::HostVisible)));
	}
}

void Mupfel::Renderer::SyncRenderableObjects(World& world, const Ping::Device& device, uint32_t frame_index)
{
	Ping::Buffer&  buffer = vertex_buffers[frame_index];
	auto*		   mapped_ptr = static_cast<Transform*>(buffer.GetMappedPtr());
	const uint32_t capacity = static_cast<uint32_t>(buffer.Size() / sizeof(Transform));

	uint32_t write_index = 0;
	for (auto [e, transform, texture] : world.registry.view<Transform, Texture>())
	{
		if (write_index >= capacity)
			break;

		mapped_ptr[write_index++] = transform;
	}

	/* data is now moved into the GPU buffers */
	device.Flush(vertex_buffers[frame_index]);
}

void Mupfel::Renderer::RenderNextFrame(World& world, const Ping::Device& device, const Window& window)
{

	Ping::CommandBuffer& current_command_buffer = commandBuffers.value()[frameIndex];

	current_command_buffer.WaitForFences(device);

	SyncRenderableObjects(world, device, frameIndex);

	uint32_t image_index = swapchain.value().AcquireNextImage(frameIndex);

	/* Check if the index is valid */
	if (image_index == std::numeric_limits<uint32_t>::max())
	{
		/* Image was resized, swapchain needs to be recreated */
		swapchain.value().Recreate(device, window, frames_in_flight);
		return;
	}

	current_command_buffer.Begin(device);

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

	current_command_buffer.BindVertexBuffer(pipeline.value(), vertex_buffers[frameIndex], 0);

	current_command_buffer.Draw(3);

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
