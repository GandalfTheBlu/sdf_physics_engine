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

	Engine::SphereCollider sphere;
	sphere.position = glm::vec3(0.f, 6.f, 3.f);
	sphere.radius = 1.f;

	Engine::Rigidbody rb;
	rb.SetMass(100.f);
	rb.SetInertiaTensor(glm::mat3((2.f / 5.f) * 100.f * 1.f * 1.f));

	auto worldSDF = [](const glm::vec3& p) 
	{
		return p.y + 0.3f * glm::sin(p.x) * glm::sin(p.z);
	};

	Engine::PhysicsWorld physWorld;
	physWorld.Init(worldSDF, { 0.5f, 0.4f });
	physWorld.gravity = glm::vec3(0.f, -9.82f, 0.f);
	physWorld.AddObject(&sphere, &rb, { 1.f, 0.4f });

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
				p_obj->p_rigidbody->AddImpulseAtPoint(hit.normal * -1000.f, hit.point);
			}
		}
		

		glm::vec4 sphereData(sphere.position, sphere.radius);

		sdfShader.Use();
		sdfShader.SetMat4("u_invVP", &invVP[0][0]);
		sdfShader.SetVec3("u_camPos", &camTransform[3][0]);
		sdfShader.SetFloat("u_pixelRadius", pixelRadius);
		sdfShader.SetVec4("u_spherePosRadius", &sphereData[0]);
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
