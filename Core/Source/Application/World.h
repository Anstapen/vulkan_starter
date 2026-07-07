#pragma once

#include <vector>

#include "Core/EventSystem.h"
#include "ECS/Registry.h"

namespace Mupfel
{
/** Bundles the ECS `Registry` and the `EventSystem` it's wired to, plus the list of entities created in it. */
class World
{
public:
	/** Constructs the event system and a registry wired to it. */
	World();

public:
	/** Double-buffered event queues shared with `registry`. */
	EventSystem event_system;
	/** Component storage and views for this world's entities. */
	Registry registry;
	/** Entities created in this world (see `Application::Init`). */
	std::vector<Entity> entities;
};
} // namespace Mupfel
