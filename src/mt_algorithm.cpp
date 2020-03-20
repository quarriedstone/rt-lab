#include "mt_algorithm.h"


Sphere::Sphere(float3 center, float radius) :
	center(center), radius(radius)
{
}

Sphere::~Sphere()
{
}

IntersectableData Sphere::Intersect(const Ray& ray) const
{
	float a = dot(ray.direction, ray.direction);
	float3 oc = center - ray.position;
	float b = -2 * dot(oc, ray.direction);
	float c = dot(oc, oc) - radius * radius;
	float disc = b * b - 4 * a * c;

	if (disc < 0)
	{
		return disc;
	}

	float x0 = (-b - sqrtf(disc)) / (2.f * a);
	float x1 = (-b + sqrtf(disc)) / (2.f * a);
	float t = std::min(x0, x1);

	if (t < 0)
	{
		t = std::max(x0, x1);
	}

	return t;
}



MTAlgorithm::MTAlgorithm(short width, short height) : RayGenerationApp(width, height)
{
}

MTAlgorithm::~MTAlgorithm()
{
}

int MTAlgorithm::LoadGeometry(std::string filename)
{
	objects.push_back(new Sphere(float3{ 2, 0, -1 }, 0.4f));

	Vertex a(float3{ -0.5f, -0.5f, -1.0f });
	Vertex b(float3{ 0.5f, -0.5f, -1.0f });
	Vertex c(float3{ 0.0f, 0.5f, -1.0f });
	objects.push_back(new Triangle(a, b, c));

	return 0;
}

Payload MTAlgorithm::TraceRay(const Ray& ray, const unsigned int max_raytrace_depth) const
{
	IntersectableData closestData(t_max);
	for (auto& object : objects)
	{
		IntersectableData data = object->Intersect(ray);
		if (data.t > t_min && data.t < closestData.t)
		{
			closestData = data;
		}
	}

	if (closestData.t < t_max)
	{
		return Hit(ray, closestData);
	}

	return Miss(ray);
}

Payload MTAlgorithm::Hit(const Ray& ray, const IntersectableData& data) const
{
	Payload payload;
	payload.color = data.baricentric;
	return payload;
}

Triangle::Triangle(Vertex a, Vertex b, Vertex c) :
	a(a), b(b), c(c)
{
	this->ba = b.position - a.position;
	this->ca = c.position - a.position;
}

Triangle::Triangle() :
	a(float3{ 0, 0 ,0 }), b(float3{ 0, 0 ,0 }), c(float3{ 0, 0 ,0 })
{
}

Triangle::~Triangle()
{
}

IntersectableData Triangle::Intersect(const Ray& ray) const
{
	float3 pvec = cross(ray.direction, ca);
	float dt = dot(ba, pvec);

	if (dt > -1e-8f && dt < 1e-8f)
	{
		return IntersectableData(-1.f);
	}

	float3 tvec = ray.position - a.position;
	float u = dot(tvec, pvec) / dt;

	if (u < 0 || u > 1)
	{
		return IntersectableData(-1.f);
	}

	float3 qvec = cross(tvec, ba);
	float v = dot(ray.direction, qvec) / dt;

	if (v < 0 || u + v > 1)
	{
		return IntersectableData(-1.f);
	}

	float t = dot(ca, qvec) / dt;

	return IntersectableData(t, float3{ 1.f - u - v, u, v });
}
