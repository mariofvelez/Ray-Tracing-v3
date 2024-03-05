#pragma once

#include <glm/glm.hpp>

struct Material
{
	glm::vec3 albedo;
	float roughness;
	float metal;
};