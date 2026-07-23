#pragma once

#include <optional>
#include <random>
#include <unordered_map>

#include "Core/EventSystem.h"
#include "ECS/Registry.h"
#include "TextureManager/ImageManager.h"
#include "Ping/Device.h"

namespace Mupfel
{

/** Shared tuning constant for random entity spawning; see `World::SpawnRandomEntities`/`Application::Init`. */
constexpr float kMaxEntityVelocity = 4.0f;

class Renderer;

/** Bundles the ECS `Registry` and the `EventSystem` it's wired to, plus the list of entities created in it. */
class World
{
public:
	/** Constructs the event system and a registry wired to it. */
	World();

public:
	/**
	 * Creates `count` entities, each with a `Transform` (random position in `[min_pos, max_pos]` on
	 * X/Y) and a `Movement` (random velocity in `[-max_velocity, max_velocity]`) component.
	 */
	void SpawnRandomEntities(uint32_t count, float min_pos, float max_pos, float max_velocity);

	/**
	 * Builds the demo scene: a large grass-tiled ground quad, a player billboard (stored in
	 * `player`), and a ring of billboard props (some floating above the plane).
	 */
	void SpawnScene(Renderer& renderer, Ping::Device& device, ImageManager& manager);

	/**
	 * Advances every entity that has both a `Light` and a `Transform` around a horizontal circle
	 * on the x/y plane, framerate-independently via `delta_time`.
	 *
	 * All light entities share one orbit angle, so with more than one light they move in lockstep.
	 *
	 * @param delta_time Seconds elapsed since the previous frame.
	 */
	void UpdateLights(float delta_time);

private:
	void LoadImages(Renderer& renderer, Ping::Device &device, ImageManager& manager);

public:
	/** Double-buffered event queues shared with `registry`. */
	EventSystem event_system;
	/** Component storage and views for this world's entities. */
	Registry registry;
	/** Entities created in this world (see `Application::Init`). */
	std::vector<Entity> entities;
	/** The entity the camera follows; set by `SpawnScene`. Empty until then. */
	std::optional<Entity> player;

private:
	/** Shared RNG for `SpawnRandomEntities`. */
	std::mt19937 rng{std::random_device{}()};
	/** Accumulated orbit angle in radians for `UpdateLights`. */
	float lightOrbitAngle = 0.0f;

	std::unordered_map<std::string, ImageHandle> image_map;
};
} // namespace Mupfel
