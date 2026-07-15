#pragma once
#include "ComponentArray.h"
#include <cassert>
#include <vector>

namespace Mupfel
{

template <typename FirstComponent, typename... Components> class View;

/**
 * Host-memory, sparse-set storage for one component type `T`.
 *
 * `sparse[entity_index]` maps an entity to its slot in the parallel `dense`/`components` arrays
 * (or `IComponentArray::invalid_entry` if the entity has no component of this type). `Remove` is
 * O(1): it swaps the removed slot with the last slot in `dense`/`components`, so iteration order
 * over `components` is not stable across removals.
 */
template <typename T> class CPUComponentArray : public ComponentArray<T>
{
	template <typename FirstComponent, typename... Components> friend class View;

public:
	/** Reserves `capacity` entries up front in `sparse`, `dense`, and `components`. */
	CPUComponentArray(uint32_t capacity = 1000);
	~CPUComponentArray() override = default;

	/** Removes `e`'s component by swapping it with the last entry, if present. A no-op otherwise. */
	virtual void Remove(Entity e) override;

	virtual bool Has(Entity e) const override;

	/**
	 * Raw access to the dense entity-index array (parallel to `components`), e.g. for
	 * `View`/`Registry::ParallelForEach` iteration.
	 */
	virtual uint32_t* GetDense();

	/** Number of components currently stored. */
	virtual uint32_t Size() override;

	T&	 Get(Entity e) override;
	void Set(Entity e, T val) override;

	/** @warning Asserts `e` does not already have a component of this type. */
	void Insert(Entity e, T component) override;

private:
	/** Entity index -> slot in `dense`/`components`, or `invalid_entry`. */
	std::vector<size_t> sparse;
	/** Slot -> owning entity index; parallel to `components`. */
	std::vector<uint32_t> dense;
	/** Component data, indexed by the same slot as `dense`. */
	std::vector<T> components;
};

template <typename T> inline void CPUComponentArray<T>::Insert(Entity e, T component)
{
	/*
		If the sparse array is too small, we resize it using the invalid_index as value.
		We resize in powers of two for now.
	*/
	if (e.Index() >= sparse.size())
	{
		std::size_t new_size = (static_cast<std::size_t>(e.Index()) + 2) * 2;
		sparse.resize(new_size, IComponentArray::invalid_entry);
	}

	/*
		A value of "invalid_index" shows that the entity does not have the component yet.
		Multiple components of the same type for one entity are illegal.
	*/
	assert(sparse[e.Index()] == IComponentArray::invalid_entry && "Entity already has a component of this type!");

	/*
		New entries of the component and dense vectors are always pushed at the end.
	*/
	sparse[e.Index()] = dense.size();

	/* The value at the index stores which entity uses the component */
	dense.push_back(e.Index());
	components.push_back(component);
}

template <typename T> inline CPUComponentArray<T>::CPUComponentArray(uint32_t capacity)
{
	sparse.reserve(capacity);
	dense.reserve(capacity);
	components.reserve(capacity);
}

template <typename T> inline void CPUComponentArray<T>::Remove(Entity e)
{
	if (!Has(e))
	{
		return;
	}

	size_t comp_index = sparse[e.Index()];
	size_t last_index = dense.size() - 1;

	/* Swap the element that should be removed with the last one */
	std::swap(dense[comp_index], dense[last_index]);
	std::swap(components[comp_index], components[last_index]);

	/* the component order changed, update the sparse list */
	sparse[dense[comp_index]] = comp_index;

	/* Delete the last component */
	components.pop_back();
	dense.pop_back();

	/* invalidate the component reference */
	sparse[e.Index()] = IComponentArray::invalid_entry;
}

template <typename T> inline bool CPUComponentArray<T>::Has(Entity e) const
{
	return e.Index() < sparse.size() && sparse[e.Index()] != IComponentArray::invalid_entry;
}

template <typename T> inline uint32_t* CPUComponentArray<T>::GetDense() { return dense.data(); }

template <typename T> inline uint32_t CPUComponentArray<T>::Size() { return static_cast<uint32_t>(dense.size()); }

template <typename T> inline T& CPUComponentArray<T>::Get(Entity e)
{
	assert(Has(e) && "Given Entity does not currently have a component of this type!");

	return components[sparse[e.Index()]];
}

template <typename T> inline void CPUComponentArray<T>::Set(Entity e, T val)
{
	assert(Has(e) && "Given Entity does not currently have a component of this type!");
	components[sparse[e.Index()]] = val;
}

} // namespace Mupfel
