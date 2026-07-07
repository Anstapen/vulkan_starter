#pragma once
#include <cstdint>
#include <vector>
#include <bitset>
#include "MupfelDeps/Core/Event.h"
#include "MupfelDeps/Core/EventBuffer.h"

namespace Mupfel {

	class Entity
	{
	public:
		using Signature = std::bitset<64>;
	public:
		Entity() : index(0) {}
		Entity(uint32_t in_index) : index(in_index) {}
		uint32_t Index() const { return index; }
		bool operator==(const Entity& other) const { return index == other.index; }

	private:
		uint32_t index;
	};

	static_assert((sizeof(Entity) == 4) && "Error: Size of Entity Class is not 4 bytes!");

	class EntityCreatedEvent : public Event {
	public:
		EntityCreatedEvent() : e(0) {}
		EntityCreatedEvent(Entity in_e) : e(in_e) {};
		virtual ~EntityCreatedEvent() = default;

	public:
		Entity e;
	};

	class EntityDestroyedEvent : public Event {
	public:
		EntityDestroyedEvent() : e(0) {}
		EntityDestroyedEvent(Entity in_e) : e(in_e) {};
		virtual ~EntityDestroyedEvent() = default;

	public:
		Entity e;
	};


	class EntityManager {
	public:
		EntityManager() : freeList(), current_entities(0), next_entity_index(1) { freeList.reserve(4096); }
		Entity CreateEntity();
		void DestroyEntity(Entity e);
		uint32_t GetCurrentEntities() const;
	private:
		uint32_t current_entities;
		uint32_t next_entity_index;
		std::vector<uint32_t> freeList;
	};

}