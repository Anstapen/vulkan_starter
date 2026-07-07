#pragma once
#include "Core/EventSystem.h"
#include "Entity.h"
#include <algorithm>
#include <cassert>
#include <functional>
#include <future>
#include <memory>
#include <typeindex>
#include <vector>

#include "CPUComponentArray.h"
#include "ECS/Components/ComponentIndex.h"

namespace Mupfel
{

template <typename FirstComponent, typename... Components> class View;

class CollisionSystem;
class Application;
class MovementSystem;
class RayCastSystem;

/**
 * Fired via `EventSystem::AddImmediateEvent` from `Registry::AddComponent`, after the component
 * is inserted and the entity's signature updated.
 */
class ComponentAddedEvent : public Event
{
public:
	ComponentAddedEvent() = default;

	/**
	 * @param in_e The entity the component was added to.
	 * @param in_sig The entity's signature after adding the component.
	 * @param in_comp_id `ComponentIndex::Index` of the added component type.
	 */
	ComponentAddedEvent(Entity in_e, Entity::Signature in_sig, size_t in_comp_id)
		: e(in_e), comp_id(in_comp_id), sig(in_sig) {};
	virtual ~ComponentAddedEvent() = default;

public:
	/** The entity the component was added to. */
	Entity e;
	/** The entity's signature after adding the component. */
	Entity::Signature sig;
	/** `ComponentIndex::Index` of the added component type. */
	size_t comp_id;
};

/**
 * Fired via `EventSystem::AddImmediateEvent` from `Registry::RemoveComponent`, before the
 * component is actually removed, so listeners can still inspect it.
 */
class ComponentRemovedEvent : public Event
{
public:
	ComponentRemovedEvent() = default;

	/**
	 * @param in_e The entity the component is being removed from.
	 * @param in_sig The entity's signature before the component is removed.
	 * @param in_comp_id `ComponentIndex::Index` of the component type being removed.
	 */
	ComponentRemovedEvent(Entity in_e, Entity::Signature in_sig, size_t in_comp_id)
		: e(in_e), comp_id(in_comp_id), sig(in_sig) {};
	virtual ~ComponentRemovedEvent() = default;

public:
	/** The entity the component is being removed from. */
	Entity e;
	/** The entity's signature before the component is removed. */
	Entity::Signature sig;
	/** `ComponentIndex::Index` of the component type being removed. */
	size_t comp_id;
};

/**
 * Owns all entities and components for a `World`: entity lifecycle (via `EntityManager`), one
 * `CPUComponentArray<T>` per component type `T` ever used, and each entity's `Entity::Signature`
 * (which components it has). Fires `EntityCreatedEvent`/`EntityDestroyedEvent`/`ComponentAddedEvent`/
 * `ComponentRemovedEvent` on `evt_system` as entities and components change.
 */
class Registry
{
	template <typename FirstComponent, typename... Components> friend class View;
	friend class CollisionSystem;
	friend class MovementSystem;
	friend class Renderer;
	friend class RayCastSystem;

public:
	/**
	 * Owning pointer type stored in `component_buffer`; `IComponentArray` is the type-erased base
	 * so arrays of different component types can share one vector.
	 */
	using SafeComponentArrayPtr = std::unique_ptr<IComponentArray>;

public:
	/** `in_evt_sys` is where entity/component lifecycle events are fired; the registry only borrows it. */
	Registry(EventSystem& in_evt_sys) : evt_system(in_evt_sys) {}

	/** Creates a new entity (recycling a destroyed index if one is available) and fires `EntityCreatedEvent`. */
	Entity CreateEntity();

	/** Fires `EntityDestroyedEvent`, removes `e`'s components from every component array, then recycles its index. */
	void DestroyEntity(Entity e);

	/** Number of currently-live entities. */
	uint32_t GetCurrentEntities() const;

	/**
	 * @warning Asserts `index` refers to an entity that was created via `CreateEntity`.
	 * @return The signature (which components it has) of the entity at `index`.
	 */
	Entity::Signature GetSignature(uint32_t index) const;

	/** Returns a `View` iterating every entity that has all of `Components...`. */
	template <typename... Components> View<Components...> view() { return View<Components...>(*this); }

	/**
	 * Runs `function(Entity, Components&...)` over every entity with all of `Components...`, split
	 * into chunks and dispatched across `Application::GetCurrentThreadPool()`. Entities for which
	 * `function` returns `true` are appended to `changed_entities` (order not guaranteed across threads).
	 */
	template <typename... Components, typename F>
	void ParallelForEach(F&& function, std::vector<Entity>& changed_entities);

	/** Constructs a `T` in place from `args` and adds it to `e` (see the `AddComponent(Entity, T)` overload). */
	template <typename T, typename... Args> void AddComponent(Entity e, Args&&... args);

	/**
	 * Adds `component` to `e`, updates `e`'s signature, and fires `ComponentAddedEvent`.
	 * @warning `e` must not already have a component of type `T`.
	 */
	template <typename T> void AddComponent(Entity e, T component);

	/** Fires `ComponentRemovedEvent`, removes `e`'s component of type `T`, and updates `e`'s signature. */
	template <typename T> void RemoveComponent(Entity e);

	/** @warning Undefined unless `HasComponent<T>(e)` is true. */
	template <typename T> T& GetComponent(Entity e);

	/** @warning Undefined unless `HasComponent<T>(e)` is true. */
	template <typename T> void SetComponent(Entity e, T comp);

	/** Whether `e` currently has a component of type `T`. */
	template <typename T> bool HasComponent(Entity e);

	/** The combined `Entity::Signature` bit for each of `Components...`, for signature comparisons. */
	template <typename... Components> static inline std::bitset<64> ComponentSignature()
	{
		std::bitset<64> sig;
		(sig.set(ComponentIndex::Index<Components>()), ...);
		return sig;
	}

private:
	/** Returns (creating on first use) the `CPUComponentArray<T>` for component type `T`. */
	template <typename T> CPUComponentArray<T>& GetComponentArray();

	/** Grows `component_buffer` if needed so `T`'s slot (`ComponentIndex::Index<T>()`) is valid. */
	template <typename T> void resizeComponentBuffer();

private:
	/** Where entity/component lifecycle events are fired. */
	EventSystem& evt_system;
	/** Allocates/recycles entity indices. */
	EntityManager entity_manager;
	/** Per-entity-index signature, resized alongside entities. */
	std::vector<Entity::Signature> signatures;
	/** Per-component-type storage, indexed by `ComponentIndex`. */
	std::vector<SafeComponentArrayPtr> component_buffer;
};

template <typename... Components, typename F>
inline void Registry::ParallelForEach(F&& function, std::vector<Entity>& changed_entities)
{
	auto view = this->view<Components...>();

	auto&		   pool = Application::GetCurrentThreadPool();
	const uint32_t num_threads = pool.GetThreadCount();

	using BaseComponent = std::tuple_element_t<0, std::tuple<Components...>>;
	auto&		   array = GetComponentArray<BaseComponent>();
	const auto&	   dense = array.dense;
	const uint32_t total = dense.size();

	auto arrays = std::make_tuple(&GetComponentArray<Components>()...);

	if (total == 0)
	{
		return;
	}

	const uint32_t								  chunk = (total + num_threads - 1) / num_threads;
	std::vector<std::future<std::vector<Entity>>> jobs;
	jobs.reserve(num_threads);

	const auto required = Registry::ComponentSignature<Components...>();

	for (uint32_t t = 0; t < num_threads; t++)
	{
		const uint32_t begin = t * chunk;
		const uint32_t end = std::min(begin + chunk, total);

		if (begin >= end)
		{
			continue;
		}

		jobs.push_back(pool.Enqueue(
			[this, begin, end, function, &dense, required, &arrays]() mutable -> std::vector<Entity>
			{
				std::vector<Entity> local_entities;
				local_entities.reserve(64);

				for (size_t i = begin; i < end; ++i)
				{
					Entity		e{dense[i]};
					const auto& sig = GetSignature(e.Index());

					/* check if the entity has all the needed components */
					if ((sig & required) != required)
						continue;

					/* Call given function on the entity */
					// bool entity_changed = function(e, GetComponent<Components>(e)...);

					std::apply(
						[&](auto*... arr)
						{
							bool entity_changed = function(e, arr->Get(e)...);
							if (entity_changed)
								local_entities.push_back(e);
						},
						arrays);
				}

				return local_entities;
			}));
	}

	/* Wait for all jobs to finish and collect the entities */
	for (auto& job : jobs)
	{
		auto local = job.get();
		changed_entities.insert(
			changed_entities.end(), std::make_move_iterator(local.begin()), std::make_move_iterator(local.end()));
	}
}

template <typename T, typename... Args> inline void Registry::AddComponent(Entity e, Args&&... args)
{
	AddComponent(e, T(std::forward<Args>(args)...));
}

template <typename T> inline void Registry::AddComponent(Entity e, T component)
{

	ComponentArray<T>& storage = GetComponentArray<T>();
	storage.Insert(e, component);

	/* Update the Entity Signature */
	uint32_t id = static_cast<uint32_t>(ComponentIndex::Index<T>());
	signatures[e.Index()].set(id);

	/* Send a ComponentAdded Event */
	evt_system.AddImmediateEvent<ComponentAddedEvent>({e, signatures[e.Index()], id});
}

template <typename T> inline void Registry::RemoveComponent(Entity e)
{
	uint32_t id = ComponentIndex::Index<T>();
	/* Send a ComponentRemoved Event */
	evt_system.AddImmediateEvent<ComponentRemovedEvent>({e, signatures[e.Index()], id});

	ComponentArray<T>& storage = GetComponentArray<T>();
	storage.Remove(e);

	/* Update the Entity Signature */
	signatures[e.Index()].reset(id);
}

template <typename T> inline T& Registry::GetComponent(Entity e) { return GetComponentArray<T>().Get(e); }

template <typename T> inline void Registry::SetComponent(Entity e, T comp) { GetComponentArray<T>().Set(e, comp); }

template <typename T> inline bool Registry::HasComponent(Entity e) { return GetComponentArray<T>().Has(e); }

template <typename T> inline CPUComponentArray<T>& Registry::GetComponentArray()
{
	size_t comp_index = ComponentIndex::Index<T>();

	resizeComponentBuffer<T>();

	/* Create a new Component Array for the given Type if there is none */
	if (!component_buffer[comp_index])
	{
		SafeComponentArrayPtr new_array = std::make_unique<CPUComponentArray<T>>(50000);
		component_buffer[comp_index] = std::move(new_array);
	}

	return *static_cast<CPUComponentArray<T>*>(component_buffer[comp_index].get());
}

template <typename T> inline void Registry::resizeComponentBuffer()
{
	size_t comp_index = ComponentIndex::Index<T>();

	if (comp_index >= component_buffer.size())
	{
		component_buffer.resize(comp_index + 1);
	}
}

} // namespace Mupfel

#include "ECS/View.h"