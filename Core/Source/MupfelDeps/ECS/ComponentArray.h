#pragma once
#include "Entity.h"
#include <cstdint>

namespace Mupfel
{

/**
 * Type-erased interface every `ComponentArray<T>` implements, so `Registry` can hold a
 * heterogeneous `component_buffer` of them and operate on an entity without knowing `T`.
 */
class IComponentArray
{
public:
	virtual ~IComponentArray() = default;

	/** Removes the component for `e`, if present. A no-op otherwise. */
	virtual void Remove(Entity e) = 0;

	/** Whether `e` currently has a component in this array. */
	virtual bool Has(Entity e) const = 0;

	/** Number of components currently stored. */
	virtual uint32_t Size() = 0;

	/** Sentinel used by sparse-set implementations (e.g. `CPUComponentArray`) to mark "no component". */
	static constexpr size_t invalid_entry = std::numeric_limits<size_t>::max();
};

/** Typed storage interface for one component type `T`, implemented by `CPUComponentArray<T>`. */
template <typename T> class ComponentArray : public IComponentArray
{
	template <typename FirstComponent, typename... Components> friend class View;
	friend class Registry;
	friend class MovementSystem;
	friend class Renderer;
	friend class CollisionSystem;

public:
	virtual ~ComponentArray() override = default;

	/** @warning Undefined unless `Has(e)` is true. */
	virtual T& Get(Entity e) = 0;

	/** @warning Undefined unless `Has(e)` is true. */
	virtual void Set(Entity e, T val) = 0;

	/** Adds a component for `e`. `e` must not already have one. */
	virtual void Insert(Entity e, T component) = 0;
};
} // namespace Mupfel