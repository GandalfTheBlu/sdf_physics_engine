#include "window.h"
#include "player.h"
#include <array>
#include "program_handle.h"
#include "shader.h"
#include "sdf_renderer.h"
#include "file_watcher.h"

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
	Engine::FileWatcher sdfFileWatcher;
	SdfRenderer sdfRenderer;
	Engine::PhysicsWorld physicsWorld;
	Player player;
	std::array<Sphere, 10> spheres;
	std::array<Capsule, 10> capsules;

	App_SetupTest();

	void Init();
	void UpdateLoop();
	void Deinit();

	void UpdateSdfFileWatcher();
	void ReloadWorldSdf();
};