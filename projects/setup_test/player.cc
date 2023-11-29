#include "player.h"
#include "physics_world.h"
#include "input.h"

Player::Player() :
	p_physicsWorld(nullptr),
	cameraTransform(1.f),
	movementSpeed(12.f),
	jumpHeight(2.f),
	cameraPitch(0.f),
	cameraYaw(0.f),
	sensitivity(0.003f)
{}

glm::vec3 Player::GetCameraPos()
{
	return rigidbody.centerOfMass + glm::vec3(0.f, collider.height * 0.5f, 0.f);
}

void Player::AddToPhysicsWorld(PhysicsWorld& physicsWorld)
{
	physicsWorld.AddObject(&collider, &rigidbody, { 0.1f, 0.8f });
	p_physicsWorld = &physicsWorld;
}

bool Player::IsOnGround()
{
	HitResult hit;
	return p_physicsWorld->RaycastWorld(
		rigidbody.centerOfMass, 
		glm::vec3(0.f, -1.f, 0.f), 
		collider.height * 0.5f + collider.radius + 0.3f,
		hit
	);
}

glm::vec3 ClampMagnitude(const glm::vec3& v, float m)
{
	float len = glm::length(v);
	if (len <= m)
		return v;

	return v * (m / len);
}

void Player::Update(float deltaTime)
{
	auto& IP = Engine::Input::Instance();
	const Engine::Mouse& mouse = IP.GetMouse();

	glm::vec2 axis(0.f);

	if (IP.GetKey(GLFW_KEY_W).IsDown())
		axis.y = 1.f;
	else if (IP.GetKey(GLFW_KEY_S).IsDown())
		axis.y = -1.f;

	if (IP.GetKey(GLFW_KEY_D).IsDown())
		axis.x = 1.f;
	else if (IP.GetKey(GLFW_KEY_A).IsDown())
		axis.x = -1.f;

	glm::vec3 planarRight= glm::normalize(glm::vec3(cameraTransform[0].x, 0.f, cameraTransform[0].z));
	glm::vec3 planarForward = glm::normalize(glm::vec3(cameraTransform[2].x, 0.f, cameraTransform[2].z));

	float velY = rigidbody.linearVelocity.y;
	glm::vec3 move = planarRight * axis.x + planarForward * axis.y;

	rigidbody.linearVelocity = ClampMagnitude(move, 1.f) * movementSpeed + glm::vec3(0.f, velY, 0.f);

	if (IP.GetKey(GLFW_KEY_SPACE).WasPressed() && IsOnGround())
		rigidbody.linearVelocity.y = glm::sqrt(2.f * 9.82f * jumpHeight);

	static bool mouseIsVisible = false;

	if (IP.GetKey(GLFW_KEY_ESCAPE).WasPressed())
		mouseIsVisible = !mouseIsVisible;

	if (!mouseIsVisible)
	{
		cameraYaw += (float)mouse.movement.dx * sensitivity;
		cameraPitch += (float)mouse.movement.dy * sensitivity;
	}

	cameraPitch = glm::clamp(cameraPitch, -1.5f, 1.5f);

	glm::vec3 camPos = GetCameraPos();
	glm::quat camRotation = glm::quat(glm::vec3(cameraPitch, cameraYaw, 0.f));

	cameraTransform = glm::mat4(1.f);
	cameraTransform[3] = glm::vec4(camPos, 1.f);
	cameraTransform = cameraTransform * glm::mat4_cast(camRotation);

	glm::vec3 camForward = cameraTransform[2];

	if (mouse.leftButton.WasPressed())
	{
		Engine::HitResult hit;
		Engine::PhysicsObject* p_obj = p_physicsWorld->RaycastObjects(camPos, camForward, 100.f, hit, &collider);
		if (p_obj != nullptr)
		{
			p_obj->p_rigidbody->AddImpulseAtPoint(camForward * 500.f, hit.point);
		}
	}
}