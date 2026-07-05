#include "Renderer.h"
#include "Ping/Types.h"

using namespace Mupfel;

void Mupfel::Renderer::Init(const Ping::Device& device, const Window& window)
{
	logger = Logger::Create("Renderer");
	logger->info("Init");
	swapchain = device.CreateSwapChain(window, frames_in_flight);
	pipeline = device.CreatePipeline(Ping::PipelineSpecification{ "Shaders/slang.spv" }, swapchain.value());
	commandBuffers = device.CreateCommandBuffers(Ping::QueueType::Graphics, frames_in_flight);
	assert(commandBuffers.has_value() && commandBuffers.value().size() > 0);
}

void Mupfel::Renderer::RenderNextFrame(const Ping::Device& device, const Window& window)
{
	Ping::CommandBuffer& current_command_buffer = commandBuffers.value()[frameIndex];

	
	current_command_buffer.WaitForFences(device);

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
		.dstStage = Ping::PipelineStage::ColorAttachmentOutput
	};

	current_command_buffer.transitionImageLayout(swapchain.value(), image_index, layout_transition);

	current_command_buffer.BeginRendering(swapchain.value(), image_index);

	current_command_buffer.BindPipeline(pipeline.value());

	current_command_buffer.Draw();

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

void Mupfel::Renderer::Shutdown()
{
}

void Mupfel::Renderer::incrementFrameIndex()
{
	frameIndex = (frameIndex + 1) % frames_in_flight;
}
