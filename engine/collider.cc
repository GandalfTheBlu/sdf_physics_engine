#include "collider.h"
#include <limits>
#include <glm.hpp>

namespace Engine
{
	AABB::AABB() :
		min(std::numeric_limits<float>::max()),
		max(-std::numeric_limits<float>::max())
	{}

	Collider::Collider() :
		localMatrix(1.f),
		worldMatrix(1.f)
	{}

	SphereCollider::SphereCollider() :
		radius(1.f)
	{
		sdf = [this](const glm::vec3& p)
		{
			glm::vec3 position = this->worldMatrix[3];
			return glm::distance(p, position) - this->radius;
		};
	}

	void SphereCollider::UpdateWorldAABB()
	{
		glm::vec3 position = worldMatrix[3];
		worldAABB.min = position - glm::vec3(radius);
		worldAABB.max = position + glm::vec3(radius);
	}

	bool SphereCollider::IntersectsSDF(const SDF& otherSDF, HitResult& outHitReslut) const
	{
		glm::vec3 position = worldMatrix[3];
		float dist = otherSDF(position);

		if (dist > radius)
			return false;

		outHitReslut.normal = CalcNormal(otherSDF, position);
		outHitReslut.point = position - outHitReslut.normal * dist;
		outHitReslut.distance = radius - dist;// distance = overlap
		return true;
	}

	bool SphereCollider::IntersectsRay(const glm::vec3& origin, const glm::vec3& direction, HitResult& outHitResult) const
	{
		glm::vec3 center = worldMatrix[3];
		glm::vec3 oc = origin - center;
		float a = glm::dot(direction, direction);
		float b = 2.f * glm::dot(oc, direction);
		float c = glm::dot(oc, oc) - radius * radius;
		float discr = b * b - 4.f * a * c;
		
		if (discr < 0)
			return false;

		outHitResult.distance = (-b - glm::sqrt(discr)) / (2.f * a);
		outHitResult.point = origin + direction * outHitResult.distance;
		outHitResult.normal = glm::normalize(outHitResult.point - center);
		return true;
	}


	CapsuleCollider::CapsuleCollider() :
		radius(1.f),
		height(2.f)
	{
		sdf = [this](const glm::vec3& p)
		{
			float h0 = height * 0.5f;
			glm::vec3 a = this->worldMatrix * glm::vec4(0.f, h0, 0.f, 1.f);
			glm::vec3 b = this->worldMatrix * glm::vec4(0.f, -h0, 0.f, 1.f);
			glm::vec3 pa = p - a;
			glm::vec3 ba = b - a;
			float h = glm::clamp(glm::dot(pa, ba) / glm::dot(ba, ba), 0.f, 1.f);
			return glm::length(pa - ba * h) - this->radius;
		};
	}

	void CapsuleCollider::UpdateWorldAABB()
	{
		float h0 = height * 0.5f;
		glm::vec3 a = this->worldMatrix * glm::vec4(0.f, h0, 0.f, 1.f);
		glm::vec3 b = this->worldMatrix * glm::vec4(0.f, -h0, 0.f, 1.f);
		worldAABB.min = glm::min(a, b) - glm::vec3(radius);
		worldAABB.max = glm::max(a, b) + glm::vec3(radius);
	}

	bool CapsuleCollider::IntersectsSDF(const SDF& otherSDF, HitResult& outHitReslut) const
	{
		float h0 = height * 0.5f;
		glm::vec3 a = this->worldMatrix * glm::vec4(0.f, h0, 0.f, 1.f);
		glm::vec3 b = this->worldMatrix * glm::vec4(0.f, -h0, 0.f, 1.f);
		glm::vec3 ab = b - a;

		float t = 0.f;
		const float alpha = 0.5f;
		const float step = alpha * height / (2.f * radius);
		glm::vec3 closestPoint(0.f);
		float closestDist = radius;
		bool hit = false;
		bool breakNext = false;

		while (true)
		{
			glm::vec3 point = a + ab * t;
			
			float dist = otherSDF(point);
			if (dist < closestDist)
			{
				closestPoint = point;
				closestDist = dist;
				hit = true;
			}

			if (breakNext)
				break;

			t += step;
			if (t >= 1.f)
			{
				t = 1.f;
				breakNext = true;
			}
		}

		if (!hit)
			return false;

		outHitReslut.normal = CalcNormal(otherSDF, closestPoint);
		outHitReslut.point = closestPoint - outHitReslut.normal * closestDist;
		outHitReslut.distance = -sdf(outHitReslut.point);// distance = overlap

		return true;
	}

	bool CapsuleCollider::IntersectsRay(const glm::vec3& origin, const glm::vec3& direction, HitResult& outHitResult) const
	{
		float h0 = height * 0.5f;
		glm::vec3 pa = this->worldMatrix * glm::vec4(0.f, h0, 0.f, 1.f);
		glm::vec3 pb = this->worldMatrix * glm::vec4(0.f, -h0, 0.f, 1.f);

		glm::vec3 ba = pb - pa;
		glm::vec3 oa = origin - pa;

		float baba = glm::dot(ba, ba);
		float bard = glm::dot(ba, direction);
		float baoa = glm::dot(ba, oa);
		float rdoa = glm::dot(direction, oa);
		float oaoa = glm::dot(oa, oa);

		float a = baba - bard * bard;
		float b = baba * rdoa - baoa * bard;
		float c = baba * oaoa - baoa * baoa - radius * radius * baba;
		float h = b * b - a * c;

		float dist = 0.f;

		if (h < 0.f)
			return false;

		float t = (-b - sqrt(h)) / a;
		float y = baoa + t * bard;

		if (y > 0.0 && y < baba)
		{
			dist = t;
		}
		else
		{
			glm::vec3 oc = (y <= 0.0) ? oa : origin - pb;
			b = glm::dot(direction, oc);
			c = glm::dot(oc, oc) - radius * radius;
			h = b * b - c;

			if (h > 0.0)
			{
				dist = -b - sqrt(h);
			}
			else
				return false;
		}

		outHitResult.distance = dist;
		outHitResult.point = origin + direction * dist;
		
		glm::vec3 pa2 = outHitResult.point - pa;
		float h2 = glm::clamp(glm::dot(pa2, ba) / baba, 0.f, 1.f);
		outHitResult.normal = (pa2 - h2 * ba) / radius;

		return true;
	}
}