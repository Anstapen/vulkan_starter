#pragma once

#include <cstdint>
#include <optional>

#include "Application/World.h"
#include "Logger/Logger.h"
#include "Ping/Buffer.h"
#include "Ping/DescriptorSets.h"
#include "Ping/Device.h"
#include "Ping/Gui.h"
#include "Ping/Image.h"
#include "Ping/Sampler.h"
#include "Window/Window.h"

#include "glm/glm.hpp"

namespace Mupfel
{

struct UniformBufferObject
{
	glm::mat4 view;
	glm::mat4 proj;
};

/**
 * Drives per-frame rendering on top of the `Ping` RHI: owns the swapchain, pipeline, command
 * buffers, and one vertex buffer per frame in flight, and syncs `Transform` components from the
 * `World` into those buffers each frame.
 */
class Renderer
{
public:
	/** Creates the swapchain, pipeline, command buffers, and per-frame-in-flight vertex buffers for `window`. */
	void Init(const Ping::Device& device, const Window& window);

	/**
	 * Renders and presents one frame: waits for the current frame-in-flight slot, syncs renderable
	 * entities to its vertex buffer, records and submits the draw, and presents. Recreates the
	 * swapchain (and returns early, skipping the rest of the frame) if it's found to be out of date.
	 */
	void RenderNextFrame(World& world, const Ping::Device& device, const Window& window, float delta_time);

	/** Currently a no-op; resource cleanup happens via RAII on destruction. */
	void Shutdown();

private:
	/** Advances `frameIndex` to the next frame-in-flight slot, wrapping at `frames_in_flight`. */
	void incrementFrameIndex();

	/** Copies every `Transform` in `world` into `vertex_buffers[frame_index]`'s mapped memory and flushes it. */
	void SyncRenderableObjects(World& world, const Ping::Device& device, uint32_t frame_index);

	/**
	 * Grows `transformBuffers` (and re-creates `transformDescriptorSets` to match) if
	 * `required_capacity` exceeds the buffers' current capacity, doubling until it fits.
	 *
	 * @note Recreates every frame-in-flight buffer at once, gated behind `device.WaitForCommands()`,
	 * since the buffers are re-synced from scratch each frame and don't need their old contents preserved.
	 */
	void EnsureTransformCapacity(const Ping::Device& device, uint32_t required_capacity);

	void EnsureLineInstanceCapacity(const Ping::Device& device, uint32_t required_capacity);
	void SyncRenderableLines(World& world, const Ping::Device& device, uint32_t frame_index);

	/**
	 * Update the Model-View-Projection matricies for a specific uniform buffer.
	 */
	void updateMVP(Ping::Buffer& uniform_buffer);

	/**
	 * Updates the orbit camera from mouse and keyboard input: right-drag to rotate (yaw/pitch),
	 * scroll to zoom (distance), WASD to pan `cameraTarget` across the X/Y plane at a speed
	 * independent of framerate (scaled by `delta_time`). Skips rotation input while ImGui wants the
	 * mouse (e.g. dragging an ImGui window).
	 */
	void UpdateCamera(const Window& window, float delta_time);

	/** Draws the "Line Spawner" ImGui window and handles its Add/Clear buttons. */
	void DrawLineSpawnerUI(World& world);

private:
	/** Number of frames pipelined in parallel. */
	static constexpr uint32_t frames_in_flight = 2;
	static constexpr uint32_t max_textures = 256;
	/** Current frame-in-flight slot, in `[0, frames_in_flight)`. */
	uint32_t frameIndex = 0;
	/** Logger created in `Init`. */
	Logger::SafeLoggerPtr logger;
	/** Empty until `Init` runs. */
	std::optional<Ping::SwapChain> swapchain;
	/** Empty until `Init` runs. */
	std::optional<Ping::Pipeline> pipeline;
	/** One per frame in flight; empty until `Init` runs. */
	std::optional<Ping::CommandBuffers> commandBuffers;
	/** One host-visible vertex buffer per frame in flight. */
	std::vector<Ping::Buffer> vertex_buffers;
	/** One device-local index buffer */
	std::optional<Ping::Buffer> index_buffer;
	/** One uniform buffer per frame in flight */
	std::vector<Ping::Buffer> uniformBuffers;
	/** One storage buffer per frame in flight for the transform data */
	std::vector<Ping::Buffer> textureInstanceBuffers;
	/** Entity capacity each buffer in `transformBuffers` currently has room for; grown by `EnsureTransformCapacity`. */
	uint32_t transformCapacity = 0;
	/** Descriptor sets */
	std::optional<Ping::DescriptorSets> descriptorSets;

	std::vector<Ping::Image>			images;
	std::vector<Ping::Sampler>			samplers;
	std::optional<Ping::DescriptorSets> samplerDescriptorSets;

	std::optional<Ping::DescriptorSets> transformDescriptorSets;

	std::optional<Ping::Gui> gui;

	static constexpr uint32_t uboSetIndex = 0;
	static constexpr uint32_t samplerSetIndex = 1;
	static constexpr uint32_t transformSetIndex = 2;

	uint32_t drawable_entities = 0;

	/** Orbit camera state, in spherical coordinates around `cameraTarget`; initial values reproduce
	 * the previous fixed eye position of (2, 2, 2) relative to the origin. */
	float cameraYaw = glm::radians(45.0f);
	float cameraPitch = glm::radians(35.264f);
	float cameraDistance = 3.4641f;
	/** Point the camera orbits around; panned across the X/Y plane by WASD in `UpdateCamera`. */
	glm::vec3 cameraTarget = glm::vec3(0.0f);
	/** Whether the right mouse button was held during the previous frame's `UpdateCamera` call. */
	bool cameraDragging = false;
	/** Cursor position recorded during the previous frame's `UpdateCamera` call. */
	double lastCursorX = 0.0, lastCursorY = 0.0;

	/** "Line Spawner" ImGui window state; see DrawLineSpawnerUI. */
	glm::vec2 lineSpawnStart = glm::vec2(0.0f, 0.0f);
	glm::vec2 lineSpawnEnd = glm::vec2(1.0f, 0.0f);
	float	  lineSpawnZ = 0.0f;
	float	  lineSpawnWidth = 0.05f;
	float	  lineSpawnColor[4] = {1.0f, 1.0f, 1.0f, 1.0f};

	static constexpr uint32_t lineUboSetIndex = 0;
	static constexpr uint32_t lineInstanceSetIndex = 1;

	std::optional<Ping::Pipeline>		linePipeline;
	std::optional<Ping::Buffer>			lineVertexBuffer;
	std::optional<Ping::Buffer>			lineIndexBuffer;
	std::optional<Ping::DescriptorSets> lineUboDescriptorSets;

	/** One storage buffer per frame in flight for the LineInstance data. */
	std::vector<Ping::Buffer>			lineInstanceBuffers;
	uint32_t							lineInstanceCapacity = 0;
	std::optional<Ping::DescriptorSets> lineInstanceDescriptorSets;

	uint32_t drawable_lines = 0;
};

} // namespace Mupfel
