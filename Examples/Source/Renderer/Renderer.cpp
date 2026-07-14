#include "Renderer.h"
#include "Ping/Types.h"

#include "ECS/Components/Texture.h"
#include "ECS/Components/Transform.h"

#include <GLFW/glfw3.h>
#include <chrono>
#include <functional>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>

using namespace Mupfel;

struct Vertex
{
	/** 2D position, bound to vertex shader location 0. */
	glm::vec2 pos;
	/** RGB color, bound to vertex shader location 1. */
	glm::vec2 texCoord;

	/** The `Ping::VertexBinding` matching `Transform`'s memory layout, for `PipelineSpecification::vertexLayout`. */
	static Ping::VertexBinding GetVertexLayout()
	{
		return {
			.binding = 0,
			.stride = sizeof(Vertex),
			.inputRate = Ping::VertexInputRate::Vertex,
			.attributes = {
				{0, Ping::VertexFormat::Float32x2, offsetof(Vertex, pos)},
				{1, Ping::VertexFormat::Float32x2, offsetof(Vertex, texCoord)}}};
	}
};

static const std::vector<Vertex> vertices = {
	{{-0.5f, -0.5f}, {1.0f, 0.0f}},
	{{0.5f, -0.5f}, {0.0f, 0.0f}},
	{{0.5f, 0.5f}, {0.0f, 1.0f}},
	{{-0.5f, 0.5f}, {1.0f, 1.0f}}};

static const std::vector<uint16_t> indices = {0, 1, 2, 2, 3, 0};

static const std::string defaul_image_path = "Images/texture.jpg";

static const uint32_t default_entity_capacity = 100000;

void Mupfel::Renderer::Init(const Ping::Device& device, const Window& window)
{
	logger = Logger::Create("Renderer");
	logger->info("Init");
	swapchain = device.CreateSwapChain(window.GetGLFWHandle(), frames_in_flight);
	Ping::PipelineSpecification pipeline_spec{
		"Shaders/slang.spv",
		Vertex::GetVertexLayout(),
		{{.set = uboSetIndex,
		  .binding = 0,
		  .type = Ping::DescriptorType::UniformBuffer,
		  .stageFlags = Ping::ShaderStage::Vertex},
		 {.set = samplerSetIndex,
		  .binding = 0,
		  .type = Ping::DescriptorType::CombinedImageSampler,
		  .stageFlags = Ping::ShaderStage::Fragment},
		 {.set = transformSetIndex,
		  .binding = 0,
		  .type = Ping::DescriptorType::StorageBuffer,
		  .stageFlags = Ping::ShaderStage::Vertex}}};
	pipeline = device.CreatePipeline(pipeline_spec, swapchain.value());
	commandBuffers = device.CreateCommandBuffers(Ping::QueueType::Graphics, frames_in_flight);
	assert(commandBuffers.has_value() && commandBuffers.value().size() > 0);

	/* We need one vertex buffer for each frame in flight */
	for (uint32_t i = 0; i < frames_in_flight; i++)
	{
		/* Create vertex buffers. These hold the 4 vertices used to draw the quad. */
		auto& buffer = vertex_buffers.emplace_back(device.CreateBuffer(
			sizeof(Vertex) * vertices.size(), Ping::BufferUsage::VertexBuffer,
			Ping::MemoryProperty::HostVisible | Ping::MemoryProperty::HostCoherent |
				Ping::MemoryProperty::DeviceLocal));
		auto* mapped_ptr = static_cast<Vertex*>(buffer.GetMappedPtr());

		/* Copy vertices */
		std::memcpy(mapped_ptr, vertices.data(), buffer.Size());

		/* Create uniform buffers for the MVP matrices */
		uniformBuffers.emplace_back(device.CreateBuffer(
			sizeof(UniformBufferObject), Ping::BufferUsage::UniformBuffer,
			Ping::MemoryProperty::HostVisible | Ping::MemoryProperty::HostCoherent |
				Ping::MemoryProperty::DeviceLocal));

		/* Create transform buffers to render entities */
		transformBuffers.emplace_back(device.CreateBuffer(
			sizeof(Mupfel::Transform) * default_entity_capacity, Ping::BufferUsage::StorageBuffer,
			Ping::MemoryProperty::HostVisible | Ping::MemoryProperty::HostCoherent |
				Ping::MemoryProperty::DeviceLocal));
	}

	index_buffer = std::move(device.CreateBuffer(
		sizeof(uint16_t) * indices.size(), Ping::BufferUsage::IndexBuffer | Ping::BufferUsage::TransferDst,
		Ping::MemoryProperty::DeviceLocal));

	/* Copy indices */
	index_buffer.value().CopyHostData(device, indices.data(), sizeof(uint16_t) * indices.size());

	/* Create descriptor sets */
	descriptorSets = device.CreateDescriptorSets(pipeline.value(), uboSetIndex, uniformBuffers);

	gui = device.CreateGui(window.GetGLFWHandle(), swapchain.value(), frames_in_flight);

	/* Try to open a default image */
	std::optional<Ping::Image> defaul_image = device.CreateImage(defaul_image_path, Ping::ImageUsage::Sampled);

	if (!defaul_image.has_value())
	{
		logger->warn("Unable to load {}.", defaul_image_path);
		return;
	}

	images.push_back(std::move(defaul_image.value()));
	samplers.push_back(device.CreateSampler(
		{.filterMode = Ping::SamplerFilterMode::Linear,
		 .mipmapMode = Ping::SamplerMipMapMode::Linear,
		 .addressMode = Ping::SamplerAddressMode::Repeat,
		 .anisotropyEnable = true}));

	std::vector<std::reference_wrapper<const Ping::Sampler>> sampler_refs(images.size(), samplers.front());

	samplerDescriptorSets = device.CreateSamplerDescriptorSets(pipeline.value(), samplerSetIndex, images, sampler_refs);

	transformDescriptorSets = device.CreateStorageDescriptorSets(pipeline.value(), transformSetIndex, transformBuffers);
}

void Mupfel::Renderer::SyncRenderableObjects(World& world, const Ping::Device& device, uint32_t frame_index)
{
	/* We are currently not taking any data from the CPU */
	(void)device;

	uint32_t buffer_index = 0;

	Transform* buffer = static_cast<Transform*>(transformBuffers[frame_index].GetMappedPtr());

	for (auto [e, t] : world.registry.view<Mupfel::Transform>())
	{
		buffer[buffer_index] = t;
		buffer_index++;
	}

	drawable_entities = buffer_index;
}

void Mupfel::Renderer::updateMVP(Ping::Buffer& uniform_buffer)
{
	auto [width, height] = swapchain.value().GetExtent();

	glm::vec3 eye = cameraDistance * glm::vec3(
										  glm::cos(cameraPitch) * glm::cos(cameraYaw),
										  glm::cos(cameraPitch) * glm::sin(cameraYaw), glm::sin(cameraPitch));

	UniformBufferObject ubo{};
	ubo.view = lookAt(eye, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	ubo.proj =
		glm::perspective(glm::radians(45.0f), static_cast<float>(width) / static_cast<float>(height), 0.1f, 500.0f);
	ubo.proj[1][1] *= -1;
	std::memcpy(uniform_buffer.GetMappedPtr(), &ubo, sizeof(UniformBufferObject));
}

void Mupfel::Renderer::UpdateCamera(const Window& window)
{
	double cursorX, cursorY;
	window.GetCursorPos(cursorX, cursorY);
	bool rightDown = window.GetMouseButtonDown(GLFW_MOUSE_BUTTON_RIGHT);

	if (rightDown && cameraDragging && !ImGui::GetIO().WantCaptureMouse)
	{
		constexpr float rotateSensitivity = 0.005f;
		cameraYaw -= static_cast<float>(cursorX - lastCursorX) * rotateSensitivity;
		cameraPitch += static_cast<float>(cursorY - lastCursorY) * rotateSensitivity;
		cameraPitch = glm::clamp(cameraPitch, glm::radians(-89.0f), glm::radians(89.0f));
	}
	cameraDragging = rightDown;
	lastCursorX = cursorX;
	lastCursorY = cursorY;

	constexpr float zoomSensitivity = 0.3f;
	cameraDistance -= static_cast<float>(window.ConsumeScrollDeltaY()) * zoomSensitivity;
	cameraDistance = glm::clamp(cameraDistance, 1.0f, 250.0f);
}

void Mupfel::Renderer::RenderNextFrame(World& world, const Ping::Device& device, const Window& window)
{

	Ping::CommandBuffer& current_command_buffer = commandBuffers.value()[frameIndex];

	current_command_buffer.WaitForFences(device);

	UpdateCamera(window);
	updateMVP(uniformBuffers[frameIndex]);

	SyncRenderableObjects(world, device, frameIndex);

	uint32_t image_index = swapchain.value().AcquireNextImage(frameIndex);

	/* Check if the index is valid */
	if (image_index == std::numeric_limits<uint32_t>::max())
	{
		/* Image was resized, swapchain needs to be recreated */
		swapchain.value().Recreate(device, window.GetGLFWHandle(), frames_in_flight);
		return;
	}

	gui.value().NewFrame();
	ImGui::ShowDemoWindow();

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

	current_command_buffer.BindDescriptorSet(pipeline.value(), descriptorSets.value(), frameIndex, uboSetIndex);

	if (samplerDescriptorSets.has_value())
	{
		current_command_buffer.BindDescriptorSet(pipeline.value(), samplerDescriptorSets.value(), 0, samplerSetIndex);
	}

	current_command_buffer.BindDescriptorSet(pipeline.value(), transformDescriptorSets.value(), frameIndex, transformSetIndex);

	current_command_buffer.BindVertexBuffer(vertex_buffers[frameIndex], 0);

	current_command_buffer.BindIndexBuffer(index_buffer.value());

	// current_command_buffer.Draw(3);
	current_command_buffer.DrawIndexed(static_cast<uint32_t>(indices.size()), drawable_entities);

	current_command_buffer.DrawGui(device, gui.value(), frameIndex);

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
		swapchain.value().Recreate(device, window.GetGLFWHandle(), frames_in_flight);
	}

	incrementFrameIndex();
}

void Mupfel::Renderer::Shutdown() {}

void Mupfel::Renderer::incrementFrameIndex() { frameIndex = (frameIndex + 1) % frames_in_flight; }
