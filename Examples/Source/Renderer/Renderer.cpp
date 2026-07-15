#include "Renderer.h"
#include "Ping/Types.h"

#include "ECS/Components/RenderedLine.h"
#include "ECS/Components/Texture.h"
#include "ECS/Components/Transform.h"

#include <GLFW/glfw3.h>
#include <chrono>
#include <cmath>
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

struct LineVertex
{
	glm::vec2 pos;

	static Ping::VertexBinding GetVertexLayout()
	{
		return {
			.binding = 0,
			.stride = sizeof(LineVertex),
			.inputRate = Ping::VertexInputRate::Vertex,
			.attributes = {{0, Ping::VertexFormat::Float32x2, offsetof(LineVertex, pos)}}};
	}
};

static const std::vector<LineVertex> line_vertices = {{{0.0f, -0.5f}}, {{1.0f, -0.5f}}, {{1.0f, 0.5f}}, {{0.0f, 0.5f}}};
static const std::vector<uint16_t>	 line_indices = {0, 1, 2, 2, 3, 0};

struct TextureInstance
{
	float	 pos_x = 0.0f;
	float	 pos_y = 0.0f;
	float	 pos_z = 0.0f;
	float	 _pad1;
	float	 scale_x = 1.0f;
	float	 scale_y = 1.0f;
	float	 rotation = 0.0f;
	uint32_t index = 1;
};

struct LineInstance
{
	float pos_x = 0.0f, pos_y = 0.0f, pos_z = 0.0f, _pad0 = 0.0f;
	float rotation = 0.0f, length = 1.0f, width = 0.05f, _pad1 = 0.0f;
	float r = 1.0f, g = 1.0f, b = 1.0f, a = 1.0f;
};

static const std::string defaul_image_path = "Images/ball_default.png";

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
		  .stageFlags = Ping::ShaderStage::Fragment,
		  .count = max_textures},
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
		textureInstanceBuffers.emplace_back(device.CreateBuffer(
			sizeof(TextureInstance) * default_entity_capacity, Ping::BufferUsage::StorageBuffer,
			Ping::MemoryProperty::HostVisible | Ping::MemoryProperty::HostCoherent |
				Ping::MemoryProperty::DeviceLocal));
	}
	transformCapacity = default_entity_capacity;

	index_buffer = std::move(device.CreateBuffer(
		sizeof(uint16_t) * indices.size(), Ping::BufferUsage::IndexBuffer | Ping::BufferUsage::TransferDst,
		Ping::MemoryProperty::DeviceLocal));

	/* Copy indices */
	index_buffer.value().CopyHostData(device, indices.data(), sizeof(uint16_t) * indices.size());

	/* Create descriptor sets */
	descriptorSets = device.CreateDescriptorSets(pipeline.value(), uboSetIndex, uniformBuffers);

	gui = device.CreateGui(window.GetGLFWHandle(), swapchain.value(), frames_in_flight);

	/* Try to open a default image */
	std::optional<Ping::Image> default_image = device.CreateImage(defaul_image_path, Ping::ImageUsage::Sampled);

	if (!default_image.has_value())
	{
		logger->warn("Unable to load {}.", defaul_image_path);
		return;
	}

	std::optional<Ping::Image> blue_ball = device.CreateImage("Images/ball_blue.png", Ping::ImageUsage::Sampled);

	if (!blue_ball.has_value())
	{
		logger->warn("Unable to load {}.", "Images/ball_blue.png");
		return;
	}

	std::optional<Ping::Image> green_ball = device.CreateImage("Images/ball_green.png", Ping::ImageUsage::Sampled);

	if (!green_ball.has_value())
	{
		logger->warn("Unable to load {}.", "Images/ball_green.png");
		return;
	}

	std::optional<Ping::Image> red_ball = device.CreateImage("Images/ball_red.png", Ping::ImageUsage::Sampled);

	if (!red_ball.has_value())
	{
		logger->warn("Unable to load {}.", "Images/ball_red.png");
		return;
	}

	std::optional<Ping::Image> yellow_ball = device.CreateImage("Images/ball_yellow.png", Ping::ImageUsage::Sampled);

	if (!yellow_ball.has_value())
	{
		logger->warn("Unable to load {}.", "Images/ball_yellow.png");
		return;
	}

	images.push_back(std::move(default_image.value()));
	images.push_back(std::move(blue_ball.value()));
	images.push_back(std::move(green_ball.value()));
	images.push_back(std::move(red_ball.value()));
	images.push_back(std::move(yellow_ball.value()));
	samplers.push_back(device.CreateSampler(
		{.filterMode = Ping::SamplerFilterMode::Linear,
		 .mipmapMode = Ping::SamplerMipMapMode::Linear,
		 .addressMode = Ping::SamplerAddressMode::Repeat,
		 .anisotropyEnable = true}));

	std::vector<std::reference_wrapper<const Ping::Sampler>> sampler_refs(images.size(), samplers.front());

	samplerDescriptorSets = device.CreateTextureArrayDescriptorSet(
		pipeline.value(), samplerSetIndex, max_textures, images, sampler_refs, images.front(), samplers.front());

	transformDescriptorSets =
		device.CreateStorageDescriptorSets(pipeline.value(), transformSetIndex, textureInstanceBuffers);

	Ping::PipelineSpecification line_pipeline_spec{
		"Shaders/line.spv",
		LineVertex::GetVertexLayout(),
		{{.set = lineUboSetIndex,
		  .binding = 0,
		  .type = Ping::DescriptorType::UniformBuffer,
		  .stageFlags = Ping::ShaderStage::Vertex},
		 {.set = lineInstanceSetIndex,
		  .binding = 0,
		  .type = Ping::DescriptorType::StorageBuffer,
		  .stageFlags = Ping::ShaderStage::Vertex}}};
	linePipeline = device.CreatePipeline(line_pipeline_spec, swapchain.value());

	lineVertexBuffer = device.CreateBuffer(
		sizeof(LineVertex) * line_vertices.size(), Ping::BufferUsage::VertexBuffer,
		Ping::MemoryProperty::HostVisible | Ping::MemoryProperty::HostCoherent | Ping::MemoryProperty::DeviceLocal);
	std::memcpy(lineVertexBuffer.value().GetMappedPtr(), line_vertices.data(), lineVertexBuffer.value().Size());

	lineIndexBuffer = device.CreateBuffer(
		sizeof(uint16_t) * line_indices.size(), Ping::BufferUsage::IndexBuffer | Ping::BufferUsage::TransferDst,
		Ping::MemoryProperty::DeviceLocal);
	lineIndexBuffer.value().CopyHostData(device, line_indices.data(), sizeof(uint16_t) * line_indices.size());

	/* Reuses the same uniformBuffers `updateMVP` already writes every frame � only the descriptor set
	 * (tied to linePipeline's own layout) needs to be new, not the underlying buffers. */
	lineUboDescriptorSets = device.CreateDescriptorSets(linePipeline.value(), lineUboSetIndex, uniformBuffers);
}

void Mupfel::Renderer::EnsureTransformCapacity(const Ping::Device& device, uint32_t required_capacity)
{
	if (required_capacity <= transformCapacity)
	{
		return;
	}

	uint32_t new_capacity = transformCapacity;
	while (new_capacity < required_capacity)
	{
		new_capacity *= 2;
	}

	logger->info("Growing transform buffers from {} to {} entities", transformCapacity, new_capacity);

	/* Every frame-in-flight buffer is recreated together, so no in-flight submission may still be
	 * reading the old buffers/descriptor sets we're about to destroy. */
	device.WaitForCommands();

	textureInstanceBuffers.clear();
	for (uint32_t i = 0; i < frames_in_flight; i++)
	{
		textureInstanceBuffers.emplace_back(device.CreateBuffer(
			sizeof(TextureInstance) * new_capacity, Ping::BufferUsage::StorageBuffer,
			Ping::MemoryProperty::HostVisible | Ping::MemoryProperty::HostCoherent |
				Ping::MemoryProperty::DeviceLocal));
	}

	transformDescriptorSets =
		device.CreateStorageDescriptorSets(pipeline.value(), transformSetIndex, textureInstanceBuffers);

	transformCapacity = new_capacity;
}

void Mupfel::Renderer::EnsureLineInstanceCapacity(const Ping::Device& device, uint32_t required_capacity)
{
	if (required_capacity <= lineInstanceCapacity)
		return;

	uint32_t new_capacity = lineInstanceCapacity == 0 ? 1000 : lineInstanceCapacity;
	while (new_capacity < required_capacity)
		new_capacity *= 2;

	device.WaitForCommands();

	lineInstanceBuffers.clear();
	for (uint32_t i = 0; i < frames_in_flight; i++)
	{
		lineInstanceBuffers.emplace_back(device.CreateBuffer(
			sizeof(LineInstance) * new_capacity, Ping::BufferUsage::StorageBuffer,
			Ping::MemoryProperty::HostVisible | Ping::MemoryProperty::HostCoherent |
				Ping::MemoryProperty::DeviceLocal));
	}

	lineInstanceDescriptorSets =
		device.CreateStorageDescriptorSets(linePipeline.value(), lineInstanceSetIndex, lineInstanceBuffers);

	lineInstanceCapacity = new_capacity;
}

void Mupfel::Renderer::SyncRenderableLines(World& world, const Ping::Device& device, uint32_t frame_index)
{
	/* GetCurrentEntities() over-counts (not every entity has RenderedLine), but it's a cheap O(1)
	 * upper bound � same trade-off EnsureTransformCapacity makes for transformBuffers. */
	EnsureLineInstanceCapacity(device, world.registry.GetCurrentEntities());

	LineInstance* buffer = static_cast<LineInstance*>(lineInstanceBuffers[frame_index].GetMappedPtr());

	uint32_t index = 0;
	for (auto [e, transform, line] : world.registry.view<Transform, RenderedLine>())
	{
		(void)e;
		buffer[index] = LineInstance{
			transform.pos_x, transform.pos_y, transform.pos_z, 0.0f,  transform.rotation, line.length, line.width, 0.0f,
			line.r,			 line.g,		  line.b,		   line.a};
		index++;
	}

	drawable_lines = index;
}

void Mupfel::Renderer::SyncRenderableObjects(World& world, const Ping::Device& device, uint32_t frame_index)
{
	EnsureTransformCapacity(device, world.registry.GetCurrentEntities());

	uint32_t buffer_index = 0;

	TextureInstance* buffer = static_cast<TextureInstance*>(textureInstanceBuffers[frame_index].GetMappedPtr());

	for (auto [e, transform, texture] : world.registry.view<Mupfel::Transform, Mupfel::Texture>())
	{
		buffer[buffer_index].index = texture.index;
		buffer[buffer_index].pos_x = transform.pos_x;
		buffer[buffer_index].pos_y = transform.pos_y;
		buffer[buffer_index].pos_z = transform.pos_z;
		buffer[buffer_index].rotation = transform.rotation;
		buffer[buffer_index].scale_x = transform.scale_x;
		buffer[buffer_index].scale_y = transform.scale_y;
		buffer_index++;
	}

	drawable_entities = buffer_index;
}

void Mupfel::Renderer::DrawLineSpawnerUI(World& world)
{
	ImGui::Begin("Line Spawner");

	ImGui::DragFloat2("Start (X, Y)", &lineSpawnStart.x, 0.1f);
	ImGui::DragFloat2("End (X, Y)", &lineSpawnEnd.x, 0.1f);
	ImGui::DragFloat("Z", &lineSpawnZ, 0.1f);
	ImGui::DragFloat("Width", &lineSpawnWidth, 0.005f, 0.001f, 5.0f);
	ImGui::ColorEdit4("Color", lineSpawnColor);

	if (ImGui::Button("Add Line"))
	{
		glm::vec2 delta = lineSpawnEnd - lineSpawnStart;

		Entity e = world.registry.CreateEntity();

		Transform t;
		t.pos_x = lineSpawnStart.x;
		t.pos_y = lineSpawnStart.y;
		t.pos_z = lineSpawnZ;
		t.rotation = std::atan2(delta.y, delta.x);
		world.registry.AddComponent<Transform>(e, t);

		RenderedLine line;
		line.length = glm::length(delta);
		line.width = lineSpawnWidth;
		line.r = lineSpawnColor[0];
		line.g = lineSpawnColor[1];
		line.b = lineSpawnColor[2];
		line.a = lineSpawnColor[3];
		world.registry.AddComponent<RenderedLine>(e, line);
	}

	uint32_t line_count = 0;
	for (auto [e, line] : world.registry.view<RenderedLine>())
	{
		(void) e;
		(void) line;
		line_count++;
	}
	ImGui::Text("Lines: %u", line_count);

	ImGui::SameLine();
	if (ImGui::Button("Clear All Lines"))
	{
		/* Collect first, destroy after: Registry::DestroyEntity does swap-and-pop removal on the same
		 * dense array `view<RenderedLine>()` iterates, so destroying while iterating would skip
		 * entities (View::end() re-reads the array's current size each call, per its documented note
		 * that it's only stable if the array isn't mutated mid-iteration). */
		std::vector<Entity> lines_to_remove;
		for (auto [e, line] : world.registry.view<RenderedLine>())
		{
			(void) line;
			lines_to_remove.push_back(e);
		}
		for (Entity e : lines_to_remove)
		{
			world.registry.DestroyEntity(e);
		}
	}

	ImGui::End();
}

void Mupfel::Renderer::updateMVP(Ping::Buffer& uniform_buffer)
{
	auto [width, height] = swapchain.value().GetExtent();

	glm::vec3 eye =
		cameraTarget + cameraDistance * glm::vec3(
											glm::cos(cameraPitch) * glm::cos(cameraYaw),
											glm::cos(cameraPitch) * glm::sin(cameraYaw), glm::sin(cameraPitch));

	UniformBufferObject ubo{};
	ubo.view = lookAt(eye, cameraTarget, glm::vec3(0.0f, 0.0f, 1.0f));
	ubo.proj =
		glm::perspective(glm::radians(45.0f), static_cast<float>(width) / static_cast<float>(height), 0.1f, 500.0f);
	ubo.proj[1][1] *= -1;
	std::memcpy(uniform_buffer.GetMappedPtr(), &ubo, sizeof(UniformBufferObject));
}

void Mupfel::Renderer::UpdateCamera(const Window& window, float delta_time)
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

	constexpr float zoomSensitivity = 10.0f;
	cameraDistance -= static_cast<float>(window.ConsumeScrollDeltaY()) * zoomSensitivity;
	cameraDistance = glm::clamp(cameraDistance, 1.0f, 250.0f);

	if (!ImGui::GetIO().WantCaptureKeyboard)
	{
		constexpr float panSpeed = 30.0f;
		glm::vec3		forward(glm::cos(cameraYaw), glm::sin(cameraYaw), 0.0f);
		glm::vec3		right(-forward.y, forward.x, 0.0f);

		if (window.GetKeyDown(GLFW_KEY_W))
			cameraTarget -= forward * panSpeed * delta_time;
		if (window.GetKeyDown(GLFW_KEY_S))
			cameraTarget += forward * panSpeed * delta_time;
		if (window.GetKeyDown(GLFW_KEY_D))
			cameraTarget += right * panSpeed * delta_time;
		if (window.GetKeyDown(GLFW_KEY_A))
			cameraTarget -= right * panSpeed * delta_time;
	}
}

void Mupfel::Renderer::RenderNextFrame(World& world, const Ping::Device& device, const Window& window, float delta_time)
{

	Ping::CommandBuffer& current_command_buffer = commandBuffers.value()[frameIndex];

	current_command_buffer.WaitForFences(device);

	UpdateCamera(window, delta_time);
	updateMVP(uniformBuffers[frameIndex]);

	SyncRenderableObjects(world, device, frameIndex);
	SyncRenderableLines(world, device, frameIndex);

	uint32_t image_index = swapchain.value().AcquireNextImage(frameIndex);

	/* Check if the index is valid */
	if (image_index == std::numeric_limits<uint32_t>::max())
	{
		/* Image was resized, swapchain needs to be recreated */
		swapchain.value().Recreate(device, window.GetGLFWHandle(), frames_in_flight);
		return;
	}

	gui.value().NewFrame();
	// ImGui::ShowDemoWindow();
	ImGui::ShowMetricsWindow();

	ImGui::Begin("Entity Spawner");
	ImGui::Text("Entity count: %u", world.registry.GetCurrentEntities());
	if (ImGui::Button("Spawn 5000 Entities"))
	{
		world.SpawnRandomEntities(5000, 0.0f, 150.0f, kMaxEntityVelocity);
	}
	ImGui::End();

	DrawLineSpawnerUI(world);

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

	current_command_buffer.BindDescriptorSet(
		pipeline.value(), transformDescriptorSets.value(), frameIndex, transformSetIndex);

	current_command_buffer.BindVertexBuffer(vertex_buffers[frameIndex], 0);

	current_command_buffer.BindIndexBuffer(index_buffer.value());

	// current_command_buffer.Draw(3);
	current_command_buffer.DrawIndexed(static_cast<uint32_t>(indices.size()), drawable_entities);

	current_command_buffer.BindPipeline(linePipeline.value());
	current_command_buffer.BindDescriptorSet(
		linePipeline.value(), lineUboDescriptorSets.value(), frameIndex, lineUboSetIndex);
	current_command_buffer.BindDescriptorSet(
		linePipeline.value(), lineInstanceDescriptorSets.value(), frameIndex, lineInstanceSetIndex);
	current_command_buffer.BindVertexBuffer(lineVertexBuffer.value(), 0);
	current_command_buffer.BindIndexBuffer(lineIndexBuffer.value());
	current_command_buffer.DrawIndexed(static_cast<uint32_t>(line_indices.size()), drawable_lines);

	/* DrawGui must be last: RenderGui sets its own per-draw-command scissor rects (VKManager.cpp's
	 * RenderGui) and never restores the full-framebuffer one BeginRendering set, so anything drawn
	 * after it would inherit whatever scissor rect ImGui's last draw command left behind. */
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
