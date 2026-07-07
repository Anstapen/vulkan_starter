#pragma once
#include "Entity.h"
#include <cstdint>

namespace Mupfel {

	class IComponentArray {
	public:
		virtual ~IComponentArray() = default;
		virtual void Remove(Entity e) = 0;
		virtual bool Has(Entity e) const = 0;
		virtual uint32_t Size() = 0;
		static constexpr size_t invalid_entry = std::numeric_limits<size_t>::max();
	};

	template <typename T>
	class ComponentArray : public IComponentArray
	{
		template<typename... Components> friend class View;
		friend class Registry;
		friend class MovementSystem;
		friend class Renderer;
		friend class CollisionSystem;
	public:
		virtual ~ComponentArray() override = default;
		virtual T& Get(Entity e) = 0;
		virtual void Set(Entity e, T val) = 0;
		virtual void Insert(Entity e, T component) = 0;
	};
}