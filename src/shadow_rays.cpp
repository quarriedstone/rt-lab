#include "shadow_rays.h"

ShadowRays::ShadowRays(short width, short height): Lighting(width, height)
{
}

ShadowRays::~ShadowRays()
{
}

Payload ShadowRays::TraceRay(const Ray& ray, const unsigned int max_raytrace_depth) const
{
	if (max_raytrace_depth <= 0)
	{
		return Miss(ray);
	}

	IntersectableData closestData(t_max);
	MaterialTriangle* closestTriangle = nullptr;

	for (auto& object : material_objects)
	{
		auto data = object->Intersect(ray);
		if (data.t > t_min && data.t < closestData.t)
		{
			closestData = data;
			closestTriangle = object;
		}
	}

	if (closestData.t < t_max)
	{
		return Hit(ray, closestData, closestTriangle, max_raytrace_depth);
	}

	return Miss(ray);
}


Payload ShadowRays::Hit(const Ray& ray, const IntersectableData& data, const MaterialTriangle* triangle, const unsigned int max_raytrace_depth) const
{
	if (triangle == nullptr)
	{
		return Miss(ray);
	}

	Payload payload;
	payload.color = triangle->emissive_color;

	float3 X = ray.position + ray.direction * data.t;
	float3 N = triangle->GetNormal(data.baricentric);

	for (auto light : lights)
	{
		Ray toLight(X, light->position - X);
		float toLightDistance = length(light->position - X);
		float t = TraceShadowRay(toLight, toLightDistance);
		if (fabs(t - toLightDistance) > t_min)
		{
			continue;
		}

		// Diffuse
		payload.color += light->color * triangle->diffuse_color
			* std::max(dot(N, toLight.direction), 0.f);

		// Specular
		float3 reflectionDirection = 2.f * dot(N, toLight.direction) * N - toLight.direction;
		payload.color += light->color * triangle->specular_color
			* powf(std::max(dot(ray.direction, reflectionDirection), 0.f), triangle->specular_exponent);
	}

	return payload;
}

float ShadowRays::TraceShadowRay(const Ray& ray, const float max_t) const
{
	IntersectableData closestData(max_t);

	for (auto& object : material_objects)
	{
		auto data = object->Intersect(ray);
		if (data.t > t_min && data.t < closestData.t)
		{
			return data.t;
		}
	}

	return max_t;
}

