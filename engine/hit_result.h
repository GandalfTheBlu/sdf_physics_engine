#pragma once
#include <vec3.hpp>

namespace Engine
{
	struct HitResult
	{
		glm::vec3 point;
		glm::vec3 normal;
		float distance;

		HitResult();
	};

	inline HitResult::HitResult() :
		point(0.f),
		normal(0.f),
		distance(0.f)
	{}
}