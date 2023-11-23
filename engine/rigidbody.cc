#include "rigidbody.h"

namespace Engine
{
	Rigidbody::Rigidbody() :
		centerOfMass(glm::vec3(0.f)),
		linearVelocity(glm::vec3(0.f)),
		accumulatedForce(glm::vec3(0.f)),
		inverseMass(1.f),
		linearDamping(0.2f),
		rotation(glm::identity<glm::quat>()),
		angularVelocity(glm::vec3(0.f)),
		accumulatedTorque(glm::vec3(0.f)),
		localInverseInertiaTensor(glm::identity<glm::mat3>()),
		angularDamping(0.2f),
		worldInverseInertiaTensor(glm::identity<glm::mat3>()),
		accumulatedResponseTranslation(glm::vec3(0.f)),
		constraints(0)
	{}

	void Rigidbody::AddForce(const glm::vec3& force)
	{
		accumulatedForce += force;
	}

	void Rigidbody::AddForceAtPoint(const glm::vec3& force, const glm::vec3& point)
	{
		accumulatedForce += force;
		accumulatedTorque += glm::cross(point - centerOfMass, force);
	}

	void Rigidbody::ApplyGravity(const glm::vec3& gravityAcceleration, float deltaTime)
	{
		if (inverseMass != 0.f)
			linearVelocity += gravityAcceleration * deltaTime;
	}

	void Rigidbody::AddImpulse(const glm::vec3& impulse)
	{
		linearVelocity += impulse * inverseMass;
	}

	void Rigidbody::AddImpulseAtPoint(const glm::vec3& impulse, const glm::vec3& point)
	{
		linearVelocity += impulse * inverseMass;
		angularVelocity += worldInverseInertiaTensor * glm::cross(point - centerOfMass, impulse);
	}

	void Rigidbody::AddCollisionResponseTranslation(const glm::vec3& responseTranslation)
	{
		accumulatedResponseTranslation += responseTranslation;
	}

	void Rigidbody::SetMass(float mass)
	{
		inverseMass = 1.f / mass;
	}

	void Rigidbody::SetInertiaTensor(const glm::mat3& inertiaTensor)
	{
		localInverseInertiaTensor = glm::inverse(inertiaTensor);
	}

	void Rigidbody::Integrate(float fixedDeltaTime)
	{
		if (!(constraints & 1))
		{
			// update linear velocity based on force and mass (linear acceleration)
			// remove component in response translation direction
			linearVelocity += accumulatedForce * (fixedDeltaTime * inverseMass);

			if (accumulatedResponseTranslation != glm::vec3(0.f))
			{
				glm::vec3 translationNormal = glm::normalize(accumulatedResponseTranslation);
				float velocityNormalComponent = glm::dot(translationNormal, linearVelocity);

				// if velocity and translation are opposite in direction
				if (velocityNormalComponent < 0.f)
					linearVelocity -= translationNormal * velocityNormalComponent;
			}

			// apply linear damping
			linearVelocity *= glm::pow(1.f - glm::clamp(linearDamping, 0.f, 0.999f), fixedDeltaTime);

			// update center of mass based on linear velocity and response translation
			centerOfMass += linearVelocity * fixedDeltaTime + accumulatedResponseTranslation;
		}
		else
			linearVelocity = glm::vec3(0.f);

		if (!(constraints & 2))
		{
			// convert inertia tensor to world space
			glm::mat3 rotationMatrix(glm::mat3_cast(rotation));
			worldInverseInertiaTensor = rotationMatrix * localInverseInertiaTensor * glm::transpose(rotationMatrix);

			// update angular velocity based on angular acceleration
			angularVelocity += (worldInverseInertiaTensor * accumulatedTorque) * fixedDeltaTime;

			// apply angular damping
			angularVelocity *= glm::pow(1.f - glm::clamp(angularDamping, 0.f, 0.999f), fixedDeltaTime);

			// update rotation based on angular velocity and response rotation
			rotation += (0.5f * fixedDeltaTime) * glm::quat(0.f, angularVelocity.x, angularVelocity.y, angularVelocity.z) * rotation;
			rotation = glm::normalize(rotation);
		}
		else
			angularVelocity = glm::vec3(0.f);

		// reset accumulators
		accumulatedForce = glm::vec3(0.f);
		accumulatedTorque = glm::vec3(0.f);
		accumulatedResponseTranslation = glm::vec3(0.f);
	}

	void Rigidbody::SetLockPosition(bool flag)
	{
		constraints &= ~1;
		constraints |= (flag ? 1 : 0);
	}

	void Rigidbody::SetLockRotation(bool flag)
	{
		constraints &= ~2;
		constraints |= (flag ? 2 : 0);
	}

	glm::mat3 Rigidbody::SphereInertiaTensor(float radius, float mass)
	{
		return glm::mat3((2.f / 5.f) * mass * radius * radius);
	}

	glm::mat3 Rigidbody::CylinderInertiaTensor(float radius, float height, float mass)
	{
		float r2 = radius * radius;
		float h2 = height * height;
		float a = (1.f / 12.f) * mass * h2 + 0.25 * mass * r2;
		float b = 0.5f * mass * r2;
		
		return glm::mat3(a, 0.f, 0.f, 0.f, b, 0.f, 0.f, 0.f, a);
	}

	glm::mat3 Rigidbody::BoxInertiaTensor(const glm::vec3& size, float mass)
	{
		float a = size.x * size.x;
		float b = size.y * size.y;
		float c = size.z * size.z;
		float f = mass / 12.f;

		return glm::mat3(f * (b + c), 0.f, 0.f, 0.f, f * (a + c), 0.f, 0.f, 0.f, f * (a + b));
	}
}