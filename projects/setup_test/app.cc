#include "app.h"
#include "input.h"
#include "default_meshes.h"
#include "shader.h"
#include "camera.h"
#include "physics_world.h"

App_SetupTest::App_SetupTest()
{}

void App_SetupTest::Init()
{
	window.Init(1200, 800, "setup_test");

	glClearColor(0.1f, 0.1f, 0.1f, 0.f);
}

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

void App_SetupTest::UpdateLoop()
{
	Engine::RenderMesh screenQuad;
	Engine::GenerateUnitQuad(screenQuad);
	Engine::Shader sdfShader;
	sdfShader.Reload("assets/shaders/sdf_vert.glsl", "assets/shaders/sdf_frag.glsl");
	
	Engine::Camera cam;
	cam.Init(70.f, (float)window.Width() / window.Height(), 0.3f, 500.f);
	glm::mat4 camTransform(1.f);
	glm::vec3 camPos(0.f, 3.f, -3.f);
	glm::quat camRotation(glm::identity<glm::quat>());
	glm::vec3 camMove(0.f);
	float camUpAngle = 0.f;
	float camXAngle = 0.f;
	float camUpAngleMove = 0.f;
	float camXAngleMove = 0.f;
	float sensitivity = 0.3f;

	auto worldSDF = [](const glm::vec3& p)
	{
		return glm::min(p.y, Box(RepXZ(p, glm::vec2(3.f)), glm::vec3(1.)));
	};

	Engine::PhysicsWorld physWorld;
	physWorld.Init(worldSDF, { 0.2f, 0.4f });
	physWorld.gravity = glm::vec3(0.f, -9.82f, 0.f);

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

	struct Cube
	{
		Engine::Rigidbody rb;
		Engine::CapsuleCollider colliders[4];
		float scale;
	};

	constexpr size_t sphereCount = 10;
	Sphere spheres[sphereCount];

	for (size_t i = 0; i < sphereCount; i++)
	{
		float mass = 100.f;
		float radius = 1.f;

		Engine::Rigidbody& rb = spheres[i].rb;
		rb.centerOfMass = glm::vec3((float)i * 3.f, 6.f, 6.f);
		rb.SetMass(mass);
		rb.SetInertiaTensor(Engine::Rigidbody::SphereInertiaTensor(radius, mass));

		spheres[i].collider.radius = radius;

		physWorld.AddObject({ &spheres[i].collider }, &rb, {0.8f, 0.4f});
	}

	constexpr size_t capsuleCount = 10;
	Capsule capsules[capsuleCount];

	for (size_t i = 0; i < capsuleCount; i++)
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
		
		physWorld.AddObject({ &capsules[i].collider }, &rb, { 0.8f, 0.4f });
	}

	physWorld.Start();

	float pixelRadius = 0.5f * glm::length(glm::vec2(1.f / window.Width(), 1.f / window.Height()));

	float fixedDeltaTime = 1.f / 60.f;
	float totalTime = 0.f;

	while (!window.ShouldClose())
	{
		window.BeginUpdate();

		physWorld.Update(fixedDeltaTime);
		
		auto& IP = Engine::Input::Instance();

		if (IP.GetKey(GLFW_KEY_W).IsDown())
			camMove.z = 1.f;
		else if (IP.GetKey(GLFW_KEY_S).IsDown())
			camMove.z = -1.f;
		else
			camMove.z = 0.f;

		if (IP.GetKey(GLFW_KEY_D).IsDown())
			camMove.x = 1.f;
		else if (IP.GetKey(GLFW_KEY_A).IsDown())
			camMove.x = -1.f;
		else
			camMove.x = 0.f;

		if (IP.GetKey(GLFW_KEY_Q).IsDown())
			camMove.y = 1.f;
		else if (IP.GetKey(GLFW_KEY_E).IsDown())
			camMove.y = -1.f;
		else
			camMove.y = 0.f;

		const Engine::Mouse& mouse = IP.GetMouse();
		if (mouse.rightButton.IsDown())
		{
			camUpAngleMove = (float)mouse.movement.dx * sensitivity;
			camXAngleMove = (float)mouse.movement.dy * sensitivity;
		}
		else
		{
			camUpAngleMove = 0.f;
			camXAngleMove = 0.f;
		}

		camPos +=
			(glm::vec3(camTransform[2]) * camMove.z +
				glm::vec3(camTransform[0]) * camMove.x +
				glm::vec3(0.f, 1.f, 0.f) * camMove.y) * 0.5f;

		camXAngle += camXAngleMove * 0.02f;
		camUpAngle += camUpAngleMove * 0.02f;
		camXAngle = glm::clamp(camXAngle, -1.5f, 1.5f);

		camRotation = glm::quat(glm::vec3(camXAngle, camUpAngle, 0.f));

		camTransform = glm::mat4(1.f);
		camTransform[3] = glm::vec4(camPos, 1.f);
		camTransform = camTransform * glm::mat4_cast(camRotation);

		glm::mat4 VP = cam.CalcP() * cam.CalcV(camTransform);
		glm::mat4 invVP = glm::inverse(VP);

		if (mouse.leftButton.WasPressed())
		{
			float u = (float)(mouse.position.x / window.Width());
			float v = 1.f - (float)(mouse.position.y / window.Height());
			float x = 2.f * u - 1.f;
			float y = 2.f * v - 1.f;
			glm::vec4 nearPos = invVP * glm::vec4(x, y, -1.f, 1.f);
			glm::vec4 farPos = invVP * glm::vec4(x, y, 1.f, 1.f);
			nearPos /= nearPos.w;
			farPos /= farPos.w;
			glm::vec3 direction = glm::normalize(glm::vec3(farPos - nearPos));

			Engine::HitResult hit;
			Engine::PhysicsObject* p_obj = physWorld.RaycastObjects(camPos, direction, 100.f, hit);
			if (p_obj != nullptr)
			{
				p_obj->p_rigidbody->AddImpulseAtPoint(direction * 500.f, hit.point);
			}
		}
		

		glm::vec4 spheresData[sphereCount];
		for(size_t i=0; i<sphereCount; i++)
			spheresData[i] = glm::vec4(spheres[i].rb.centerOfMass, spheres[i].collider.radius);

		glm::vec4 capsulesData[2 * capsuleCount];
		for (size_t i = 0; i < capsuleCount; i++)
		{
			float h0 = capsules[i].collider.height * 0.5f;
			glm::vec3 pa = capsules[i].collider.worldMatrix * glm::vec4(0.f, h0, 0.f, 1.f);
			glm::vec3 pb = capsules[i].collider.worldMatrix * glm::vec4(0.f, -h0, 0.f, 1.f);
			capsulesData[i * 2] = glm::vec4(pa, capsules[i].collider.radius);
			capsulesData[i * 2 + 1] = glm::vec4(pb, 0.f);
		}

		sdfShader.Use();
		sdfShader.SetMat4("u_invVP", &invVP[0][0]);
		sdfShader.SetVec3("u_camPos", &camTransform[3][0]);
		sdfShader.SetFloat("u_pixelRadius", pixelRadius);
		sdfShader.SetVec4("u_spheres", &spheresData[0][0], sphereCount);
		sdfShader.SetInt("u_sphereCount", sphereCount);
		sdfShader.SetVec4("u_capsules", &capsulesData[0][0], capsuleCount * 2);
		sdfShader.SetInt("u_capsuleCount", capsuleCount);
		screenQuad.Bind();
		screenQuad.Draw(0);
		screenQuad.Unbind();
		sdfShader.StopUsing();

		window.EndUpdate();

		totalTime += fixedDeltaTime;
	}
}

void App_SetupTest::Deinit()
{
	window.Deinit();
}
