#include "window.h"
#include "player.h"
#include <array>
#include "program_handle.h"
#include "shader.h"
#include "file_watcher.h"

class SdfObject
{
private:
	void InitProgram(Tolo::ProgramHandle& program);

public:
	Tolo::ProgramHandle* p_program;
	Engine::Shader shader;

	std::array<Engine::FileWatcher*, 3> fileWatchers;

	SdfObject();
	~SdfObject();


	void Init(const std::string& vertShaderPath, const std::string& fragShaderPath, const std::string& sdfPath);
	void Reload();
	void Update();
};

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
	SdfObject worldSdfObject;
	Engine::PhysicsWorld physicsWorld;
	Player player;
	std::array<Sphere, 10> spheres;
	std::array<Capsule, 10> capsules;

	App_SetupTest();

	void Init();
	void UpdateLoop();
	void Deinit();
};