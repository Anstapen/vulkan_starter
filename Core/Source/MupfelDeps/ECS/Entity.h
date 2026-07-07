#pragma once
#include "MupfelDeps/Core/Event.h"
#include "MupfelDeps/Core/EventBuffer.h"
#include <bitset>
#include <cstdint>
#include <vector>

namespace Mupfel
{

/**
 * A lightweight entity handle: just a 4-byte index into `Registry`'s per-entity arrays
 * (signatures, component arrays). Entities have no identity beyond this index.
 */
class Entity
{
public:
	/** Bitmask of which component types (by `ComponentIndex::Index`) an entity currently has. */
	using Signature = std::bitset<64>;

public:
	/**
	 * Constructs an entity with index 0. Not a valid "no entity" sentinel — index 0 is a real,
	 * assignable entity index.
	 */
	Entity() : index(0) {}

	/** Wraps an existing index, e.g. one produced by `EntityManager::CreateEntity`. */
	Entity(uint32_t in_index) : index(in_index) {}

	/** The raw index this entity refers to. */
	uint32_t Index() const { return index; }

	/** Whether `other` refers to the same index. */
	bool operator==(const Entity& other) const { return index == other.index; }

private:
	/** Index into `Registry`'s per-entity arrays. */
	uint32_t index;
};

static_assert((sizeof(Entity) == 4) && "Error: Size of Entity Class is not 4 bytes!");

/** Fired via `EventSystem::AddImmediateEvent` from `Registry::CreateEntity`. */
class EntityCreatedEvent : public Event
{
public:
	EntityCreatedEvent() : e(0) {}

	/** @param in_e The entity that was just created. */
	EntityCreatedEvent(Entity in_e) : e(in_e) {};
	virtual ~EntityCreatedEvent() = default;

public:
	/** The entity that was created. */
	Entity e;
};

/**
 * Fired via `EventSystem::AddImmediateEvent` from `Registry::DestroyEntity`, before its components
 * are removed, so listeners can still inspect them.
 */
class EntityDestroyedEvent : public Event
{
public:
	EntityDestroyedEvent() : e(0) {}

	/** @param in_e The entity about to be destroyed. */
	EntityDestroyedEvent(Entity in_e) : e(in_e) {};
	virtual ~EntityDestroyedEvent() = default;

public:
	/** The entity being destroyed. */
	Entity e;
};

/**
 * Allocates and recycles `Entity` indices. Destroyed indices are pushed onto a free list and
 * reused by later `CreateEntity` calls rather than growing the index space unboundedly.
 */
class EntityManager
{
public:
	EntityManager() : freeList(), current_entities(0), next_entity_index(1) { freeList.reserve(4096); }

	/** Returns a recycled index if one is free, otherwise allocates the next unused index. */
	Entity CreateEntity();

	/** Recycles `e`'s index for reuse by a future `CreateEntity` call. */
	void DestroyEntity(Entity e);

	/** Number of currently-live entities (created but not yet destroyed). */
	uint32_t GetCurrentEntities() const;

private:
	/** Number of currently-live entities. */
	uint32_t current_entities;
	/** Next never-before-used index to hand out. */
	uint32_t next_entity_index;
	/** Recycled indices from destroyed entities, reused first. */
	std::vector<uint32_t> freeList;
};

} // namespace Mupfel