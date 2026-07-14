#pragma once

#include <random>
#include <vector>

#include "Core/EventSystem.h"
#include "ECS/Registry.h"

namespace Mupfel
{

/** Shared tuning constant for random entity spawning; see `World::SpawnRandomEntities`/`Application::Init`. */
constexpr float kMaxEntityVelocity = 4.0f;

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

public:
	/** Double-buffered event queues shared with `registry`. */
	EventSystem event_system;
	/** Component storage and views for this world's entities. */
	Registry registry;
	/** Entities created in this world (see `Application::Init`). */
	std::vector<Entity> entities;

private:
	/** Shared RNG for `SpawnRandomEntities`. */
	std::mt19937 rng{std::random_device{}()};
};
} // namespace Mupfel
