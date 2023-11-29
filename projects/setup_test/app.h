#include "window.h"
#include "player.h"
#include <array>

class App_SetupTest
{
public:
	struct Sphere
	{
		Engine::Rigidbody rb;
		Engine::SphereCollider collider;
	};

	struct Capsule
	{
		Engine::Rigidbody rb;
		Engine::CapsuleCollider collider;
	};

	Engine::Window window;
	Engine::PhysicsWorld physicsWorld;
	Player player;
	std::array<Sphere, 20> spheres;
	std::array<Capsule, 10> capsules;

	App_SetupTest();

	void Init();
	void UpdateLoop();
	void Deinit();
};