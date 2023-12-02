#include "window.h"
#include "player.h"
#include <array>
#include "program_handle.h"
#include "shader.h"

void InitProgram(Tolo::ProgramHandle& program);

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
	Tolo::ProgramHandle* p_worldSdfProgram;
	Engine::PhysicsWorld physicsWorld;
	Engine::Shader sdfShader;
	Player player;
	std::array<Sphere, 10> spheres;
	std::array<Capsule, 10> capsules;

	App_SetupTest();

	void Init();
	void UpdateLoop();
	void Deinit();
};