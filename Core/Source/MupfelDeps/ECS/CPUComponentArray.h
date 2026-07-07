#pragma once
#include "ComponentArray.h"
#include <vector>
#include <cassert>

namespace Mupfel {

	template<typename... Components> class View;

	template<typename T>
	class CPUComponentArray : public ComponentArray<T>
	{
		template<typename... Components>
		friend class View;
	public:
		CPUComponentArray(uint32_t capacity = 1000);
		~CPUComponentArray() override = default;
		virtual void Remove(Entity e) override;
		virtual bool Has(Entity e) const override;
		virtual uint32_t* GetDense();
		virtual uint32_t Size() override;
		T& Get(Entity e) override;
		void Set(Entity e, T val) override;
		void Insert(Entity e, T component) override;
	private:
		std::vector<size_t> sparse;
		std::vector<uint32_t> dense;
		std::vector<T> components;
	};

	template<typename T>
	inline void CPUComponentArray<T>::Insert(Entity e, T component)
	{
		/*
			If the sparse array is too small, we resize it using the invalid_index as value.
			We resize in powers of two for now.
		*/
		if (e.Index() >= sparse.size())
		{
			sparse.resize((sparse.size() + 2) * 2, IComponentArray::invalid_entry);
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

	template<typename T>
	inline CPUComponentArray<T>::CPUComponentArray(uint32_t capacity)
	{
		sparse.reserve(capacity);
		dense.reserve(capacity);
		components.reserve(capacity);
	}

	template<typename T>
	inline void CPUComponentArray<T>::Remove(Entity e)
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

	template<typename T>
	inline bool CPUComponentArray<T>::Has(Entity e) const
	{
		return e.Index() < sparse.size() && sparse[e.Index()] != IComponentArray::invalid_entry;
	}

	template<typename T>
	inline uint32_t* CPUComponentArray<T>::GetDense()
	{
		return dense.data();
	}

	template<typename T>
	inline uint32_t CPUComponentArray<T>::Size()
	{
		return static_cast<uint32_t>(dense.size());
	}

	template<typename T>
	inline T& CPUComponentArray<T>::Get(Entity e)
	{
		assert(Has(e) && "Given Entity does not currently have a component of this type!");

		return components[sparse[e.Index()]];
	}

	template<typename T>
	inline void CPUComponentArray<T>::Set(Entity e, T val)
	{
		assert(Has(e) && "Given Entity does not currently have a component of this type!");
		components[sparse[e.Index()]] = val;
	}

}



