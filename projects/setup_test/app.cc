#include "app.h"
#include "default_meshes.h"
#include "shader.h"
#include "input.h"

App_SetupTest::App_SetupTest()
{}

float Box(const glm::vec3& p, const glm::vec3& b)
{
	glm::vec3 q = glm::abs(p) - b;
	return glm::length(glm::max(q, 0.f)) + glm::min(glm::max(q.x, glm::max(q.y, q.z)), 0.f);
}

glm::vec3 RepXZ(const glm::vec3& p, const glm::vec2& s)
{
	glm::vec2 q = s * glm::round(glm::vec2(p.x, p.z) / s);
	return p - glm::vec3(q.x, 0.f, q.y);
}

void App_SetupTest::Init()
{
	window.Init(1200, 800, "setup_test");
	window.SetMouseVisible(false);
	glClearColor(0.1f, 0.1f, 0.1f, 0.f);

	auto worldSDF = [](const glm::vec3& p)
	{
		return glm::min(p.y, Box(RepXZ(p, glm::vec2(6.f)), glm::vec3(1.f)) - 0.1f);
	};

	physicsWorld.Init(worldSDF, { 0.2f, 0.4f });
	physicsWorld.gravity = glm::vec3(0.f, -9.82f, 0.f);

	for (size_t i = 0; i < spheres.size(); i++)
	{
		float mass = 100.f;
		float radius = 1.f;

		Engine::Rigidbody& rb = spheres[i].rb;
		rb.centerOfMass = glm::vec3((float)i * 3.f, 6.f, 6.f);
		rb.SetMass(mass);
		rb.SetInertiaTensor(Engine::Rigidbody::SphereInertiaTensor(radius, mass));

		spheres[i].collider.radius = radius;

		physicsWorld.AddObject(&spheres[i].collider, &rb, { 0.8f, 0.4f });
	}

	for (size_t i = 0; i < capsules.size(); i++)
	{
		float mass = 100.f;
		float radius = 1.f;
		float height = 2.f;

		Engine::Rigidbody& rb = capsules[i].rb;
		rb.centerOfMass = glm::vec3((float)i * 3.f, 6.f, 12.f);
		rb.SetMass(mass);
		rb.SetInertiaTensor(Engine::Rigidbody::CylinderInertiaTensor(radius, height + radius, mass));
		rb.angularDamping = 0.9f;

		capsules[i].collider.radius = radius;
		capsules[i].collider.height = height;

		physicsWorld.AddObject(&capsules[i].collider, &rb, { 0.8f, 0.4f });
	}

	player.AddToPhysicsWorld(physicsWorld);
	player.rigidbody.SetMass(100.f);
	player.rigidbody.SetInertiaTensor(Engine::Rigidbody::CylinderInertiaTensor(1.f, 2.f, 100.f));
	player.rigidbody.centerOfMass = glm::vec3(0.f, 4.f, 0.f);
	player.rigidbody.SetLockRotation(true);
	player.camera.Init(70.f, (float)window.Width() / window.Height(), 0.3f, 500.f);

	physicsWorld.Start();
}

void App_SetupTest::UpdateLoop()
{
	Engine::RenderMesh screenQuad;
	Engine::GenerateUnitQuad(screenQuad);
	Engine::Shader sdfShader;
	sdfShader.Reload("assets/shaders/sdf_vert.glsl", "assets/shaders/sdf_frag.glsl");

	Engine::RenderMesh cube;
	Engine::GenerateUnitCube(cube);
	Engine::Shader flatShader;
	flatShader.Reload("assets/shaders/flat_vert.glsl", "assets/shaders/flat_frag.glsl");

	float pixelRadius = 0.5f * glm::length(glm::vec2(1.f / window.Width(), 1.f / window.Height()));

	float fixedDeltaTime = 1.f / 60.f;

	while (!window.ShouldClose())
	{
		window.BeginUpdate();

		auto& IP = Engine::Input::Instance();
		if (IP.GetKey(GLFW_KEY_ESCAPE).WasPressed())
			break;

		physicsWorld.Update(fixedDeltaTime);
		
		player.Update(fixedDeltaTime);

		glm::mat4 VP = player.camera.CalcP() * player.camera.CalcV(player.cameraTransform);
		glm::mat4 invVP = glm::inverse(VP);

		glm::vec4 spheresData[10];
		for(size_t i=0; i<spheres.size(); i++)
			spheresData[i] = glm::vec4(spheres[i].rb.centerOfMass, spheres[i].collider.radius);

		glm::vec4 capsulesData[2 * 10];
		for (size_t i = 0; i < capsules.size(); i++)
		{
			float h0 = capsules[i].collider.height * 0.5f;
			glm::vec3 pa = capsules[i].collider.worldMatrix * glm::vec4(0.f, h0, 0.f, 1.f);
			glm::vec3 pb = capsules[i].collider.worldMatrix * glm::vec4(0.f, -h0, 0.f, 1.f);
			capsulesData[i * 2] = glm::vec4(pa, capsules[i].collider.radius);
			capsulesData[i * 2 + 1] = glm::vec4(pb, 0.f);
		}

		glm::vec2 nearFar(player.camera.GetNearPlane(), player.camera.GetFarPlane());

		sdfShader.Use();
		sdfShader.SetVec2("u_nearFar", &nearFar[0]);
		sdfShader.SetMat4("u_invVP", &invVP[0][0]);
		sdfShader.SetVec3("u_camPos", &player.cameraTransform[3][0]);
		sdfShader.SetFloat("u_pixelRadius", pixelRadius);
		sdfShader.SetVec4("u_spheres", &spheresData[0][0], 10);
		sdfShader.SetInt("u_sphereCount", spheres.size());
		sdfShader.SetVec4("u_capsules", &capsulesData[0][0], 10 * 2);
		sdfShader.SetInt("u_capsuleCount", capsules.size());
		screenQuad.Bind();
		screenQuad.Draw(0);
		screenQuad.Unbind();
		sdfShader.StopUsing();

		glm::mat4 M(1.f);
		M[3] = glm::vec4(0.f, 4.f, 16.f, 1.f);
		glm::mat4 MVP = VP * M;
		glm::vec3 color(1.f, 0.5f, 0.5f);

		flatShader.Use();
		flatShader.SetMat4("u_MVP", &MVP[0][0]);
		flatShader.SetVec3("u_color", &color[0]);
		cube.Bind();
		cube.Draw(0);
		cube.Unbind();
		flatShader.StopUsing();

		window.EndUpdate();
	}
}

void App_SetupTest::Deinit()
{
	window.Deinit();
}
