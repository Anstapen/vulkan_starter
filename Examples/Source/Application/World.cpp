#include "World.h"

#include <cmath>

#include "ECS/Components/Light.h"
#include "ECS/Components/Movement.h"
#include "ECS/Components/Texture.h"
#include "ECS/Components/Transform.h"

Mupfel::World::World() : event_system(), registry(event_system) {}

void Mupfel::World::SpawnRandomEntities(uint32_t count, float min_pos, float max_pos, float max_velocity)
{
	std::uniform_real_distribution<float> pos_dist(min_pos, max_pos);
	std::uniform_real_distribution<float> velocity_dist(-max_velocity, max_velocity);
	std::uniform_real_distribution<float> texture_dist(1, 5);
	std::uniform_real_distribution<float> tilt_dist(0.01f, 0.99f);

	for (uint32_t n = 0; n < count; n++)
	{
		Entity e = registry.CreateEntity();

		Transform t;
		t.pos_x = pos_dist(rng);
		t.pos_y = pos_dist(rng);
		t.tilt = tilt_dist(rng);
		t.scale_x = 0.5f;
		t.scale_y = 0.5f;
		registry.AddComponent<Transform>(e, t);

		Texture tex;
		tex.index = static_cast<uint32_t>(texture_dist(rng));
		registry.AddComponent<Texture>(e, tex);

		Movement m;
		m.velocity_x = velocity_dist(rng);
		m.velocity_y = velocity_dist(rng);
		registry.AddComponent<Movement>(e, m);
	}
}

void Mupfel::World::SpawnScene()
{
	// Texture indices match Renderer::Init's image load order (0 default, 1-4 balls, 5 grass_1.png).
	constexpr uint32_t kGrass = 5;
	constexpr uint32_t kPlayerTex = 3;

	// Ground: one large flat quad in the x/y plane, grass tiled ~1 texture per world unit.
	{
		Entity	  e = registry.CreateEntity();
		Transform g;
		g.scale_x = 300.0f;
		g.scale_y = 300.0f;
		g.billboard = false;
		g.uvScale = 300.0f;
		registry.AddComponent<Transform>(e, g);
		registry.AddComponent<Texture>(e, Texture{kGrass});
	}

	// Player: an upright billboard at the origin; the camera follows this entity.
	{
		Entity	  e = registry.CreateEntity();
		Transform p;
		p.scale_x = 1.5f;
		p.scale_y = 2.0f;
		registry.AddComponent<Transform>(e, p);
		registry.AddComponent<Texture>(e, Texture{kPlayerTex});
		player = e;
	}

	// A ring of billboard props; every third one floats above the plane.
	for (int i = 0; i < 12; i++)
	{
		Entity	  e = registry.CreateEntity();
		Transform t;
		t.pos_x = std::cos(i * 0.523f) * 15.0f;
		t.pos_y = std::sin(i * 0.523f) * 15.0f;
		t.pos_z = (i % 3 == 0) ? 4.0f : 0.0f;

		if (i == 3)
		{
			Light l;
			l.ambientStrength = 0.1;
			l.r = 1.0f;
			l.g = 1.0f;
			l.b = 1.0f;
			registry.AddComponent<Light>(e, l);
		}

		registry.AddComponent<Transform>(e, t);
		registry.AddComponent<Texture>(e, Texture{1u + static_cast<uint32_t>(i % 4)});
	}
}

void Mupfel::World::UpdateLights(float delta_time)
{
	// Orbit the light(s) around the world origin on the x/y plane; radius matches the prop ring.
	constexpr float orbitRadius = 15.0f;
	constexpr float orbitSpeed = 1.0f; // radians per second
	constexpr float orbitHeight = 4.0f;

	lightOrbitAngle += orbitSpeed * delta_time;

	for (auto [e, transform, light] : registry.view<Transform, Light>())
	{
		(void)e;
		(void)light;
		transform.pos_x = std::cos(lightOrbitAngle) * orbitRadius;
		transform.pos_y = std::sin(lightOrbitAngle) * orbitRadius;
		transform.pos_z = orbitHeight;
	}
}
