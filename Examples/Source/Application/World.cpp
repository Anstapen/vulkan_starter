#include "World.h"

#include "ECS/Components/Movement.h"
#include "ECS/Components/Transform.h"

Mupfel::World::World() : event_system(), registry(event_system) {}

void Mupfel::World::SpawnRandomEntities(uint32_t count, float min_pos, float max_pos, float max_velocity)
{
	std::uniform_real_distribution<float> pos_dist(min_pos, max_pos);
	std::uniform_real_distribution<float> velocity_dist(-max_velocity, max_velocity);

	for (uint32_t n = 0; n < count; n++)
	{
		Entity e = registry.CreateEntity();

		Transform t;
		t.pos_x = pos_dist(rng);
		t.pos_y = pos_dist(rng);
		t.scale_x = 0.5f;
		t.scale_y = 0.5f;
		registry.AddComponent<Transform>(e, t);

		Movement m;
		m.velocity_x = velocity_dist(rng);
		m.velocity_y = velocity_dist(rng);
		registry.AddComponent<Movement>(e, m);
	}
}
