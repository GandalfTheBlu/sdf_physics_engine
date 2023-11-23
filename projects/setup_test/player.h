#pragma once
#include "camera.h"
#include "physics_world.h"

using namespace Engine;

class Player
{
private:
	PhysicsWorld* p_physicsWorld;

	glm::vec3 GetCameraPos();

public:
	Camera camera;
	glm::mat4 cameraTransform;
	Rigidbody rigidbody;
	CapsuleCollider collider;
	float movementSpeed;
	float jumpHeight;
	float cameraPitch;
	float cameraYaw;
	float sensitivity;

	Player();

	void AddToPhysicsWorld(PhysicsWorld& physicsWorld);
	bool IsOnGround();
	void Update(float deltaTime);
};