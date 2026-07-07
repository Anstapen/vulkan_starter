#include "Registry.h"
#include <cassert>


using namespace Mupfel;

Entity Registry::CreateEntity() {

	Entity e = entity_manager.CreateEntity();

	/* Update the Component Signature of the Entity */
	if (signatures.size() <= e.Index()) [[unlikely]]
	{
		signatures.resize((signatures.size() + 1) * 2, Entity::Signature(0x0));
	} 
	else {
		signatures[e.Index()] = 0x0;
	}

	/* Entity is created successfully, notify everyone */
	evt_system.AddImmediateEvent<EntityCreatedEvent>(e);

	return e;

}

void Registry::DestroyEntity(Entity e) {
	/* Create an Entity Destroyed Event to give all Listeners time to react */
	evt_system.AddImmediateEvent<EntityDestroyedEvent>(e);

	/* We have to remove the entity from all component lists */
	for (auto&  storage : component_buffer)
	{
		if (storage)
		{
			storage->Remove(e);
		}
		
	}

	entity_manager.DestroyEntity(e);
	signatures[e.Index()].reset();
}

uint32_t Registry::GetCurrentEntities() const {
	return entity_manager.GetCurrentEntities();
}

Entity::Signature Registry::GetSignature(uint32_t index) const
{
	assert((index < signatures.size()) && "Given Entity was not created correctly!");

	return signatures[index];
}
