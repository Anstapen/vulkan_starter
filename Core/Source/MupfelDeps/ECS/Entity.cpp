#include "Entity.h"
#include <iostream>

using namespace Mupfel;

Entity EntityManager::CreateEntity()
{
    uint32_t index;

    if (!freeList.empty())
    {
        /*
            Use recycled indices if available.
        */
        index = freeList.back();
        freeList.pop_back();
    }
    else {
        /*
            No recycled indices left, use a new one.
        */
        index = next_entity_index;
        next_entity_index++;
    }
    
    current_entities++;

    return Entity(index);
}

void Mupfel::EntityManager::DestroyEntity(Entity e)
{

    /*
        Add the entity index to the recycled indices.
    */
    freeList.push_back(e.Index());

    /* Update the Entities count */
    current_entities--;
}

uint32_t Mupfel::EntityManager::GetCurrentEntities() const
{
    return current_entities;
}
