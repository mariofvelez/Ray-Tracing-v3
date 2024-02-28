#version 430 core

out vec4 FragColor;

in vec2 TexCoord;

uniform mat4 camera;
uniform vec3 camera_pos;

struct Node
{
	int axis;
	int left;
	int right;
	int tri_index;
	vec3 min;
	vec3 max;
};

layout(std430, binding = 0) buffer sceneBuffer
{
	vec3 vertices[100000];
	int indices[100000];
	int vertices_size;
	int indices_size;
};

layout(std430, binding = 1) buffer bvhBuffer
{
	Node nodes[];
};

vec3 reflect(vec3 vec, vec3 normal)
{
	vec3 n = normalize(normal);
	float dot = dot(n, vec);
	vec3 temp = n * 2 * dot;
	return vec - temp;
}

struct Ray
{
	vec3 start;
	vec3 dir;
	vec3 inv;
	vec3 col;
	bool terminate;
};

//struct Sphere
//{
//	vec3 pos;
//	float radius;
//	vec3 col;
//};
//
struct Plane
{
	vec3 pos;
	vec3 normal;
	vec3 col;
};

//float intersect(Ray ray, Sphere sphere)
//{
//	float a = dot(ray.dir, ray.dir);
//	float b = 2 * dot(ray.start - sphere.pos, ray.dir);
//	float c = dot(ray.start - sphere.pos, ray.start - sphere.pos) - sphere.radius * sphere.radius;
//	float desc = b * b - 4 * a * c;
//	if (desc < 0.0)
//		return 0.0;
//	float t = (-b - sqrt(desc)) / (2 * a);
//	if (t > 0.00001)
//		return t;
//	return 0.0;
//}
//
float intersect(Ray ray, Plane plane)
{
	float t = dot(plane.normal, plane.pos - ray.start) / dot(plane.normal, ray.dir);
	if (t > 0.00001)
		return t;
	return 0.0;
}

float intersect(Ray ray, int tri)
{
	vec3 a = vertices[indices[tri]];
	vec3 b = vertices[indices[tri + 1]];
	vec3 c = vertices[indices[tri + 2]];

	vec3 e1 = b - a;
	vec3 e2 = c - a;

	vec3 ray_cross_e2 = cross(ray.dir, e2);

	float det = dot(e1, ray_cross_e2);

	if (det > -0.00001 && det < 0.00001f)
		return 0.0;

	float inv_det = 1.0 / det;
	vec3 s = ray.start - a;

	float u = inv_det * dot(s, ray_cross_e2);

	if (u < 0 || u > 1)
		return 0.0;

	vec3 s_cross_e1 = cross(s, e1);

	float v = inv_det * dot(ray.dir, s_cross_e1);

	if (v < 0 || u + v > 1)
		return 0.0;

	float t = inv_det * dot(e2, s_cross_e1);

	return t > 0.00001 ? t : 0.0;
}

bool intersect(Ray ray, Node aabb)
{
	float tx1 = (aabb.min.x - ray.start.x) * ray.inv.x;
	float tx2 = (aabb.max.x - ray.start.x) * ray.inv.x;

	float tmin = min(tx1, tx2);
	float tmax = max(tx1, tx2);

	float ty1 = (aabb.min.y - ray.start.y) * ray.inv.y;
	float ty2 = (aabb.max.y - ray.start.y) * ray.inv.y;

	tmin = max(tmin, min(ty1, ty2));
	tmax = min(tmax, max(ty1, ty2));

	float tz1 = (aabb.min.z - ray.start.z) * ray.inv.z;
	float tz2 = (aabb.max.z - ray.start.z) * ray.inv.z;

	tmin = max(tmin, min(tz1, tz2));
	tmax = min(tmax, max(tz1, tz2));

	return tmax > tmin;
}

//uniform Sphere spheres[100];
const Plane plane = Plane(vec3(0.0, 0.0, 0.0), vec3(0.0, 0.0, 1.0), vec3(0.2, 0.2, 0.2));
uniform int num_nodes;

Ray traceRay(Ray ray)
{
	float dist = 999999.9;
	vec3 col = vec3(1.0);// vec3(0.4, 0.8, 1.0);
	vec3 normal = vec3(0.0, 0.0, 1.0);
	bool terminate = true;
	/*for (int i = 0; i < 100; ++i)
	{
		float d = intersect(ray, spheres[i]);
		if (d != 0.0 && d < dist)
		{
			dist = d;
			col =  spheres[i].col;
			normal = (ray.start + ray.dir * d) - spheres[i].pos;
			terminate = false;
		}
	}*/
	/*for (int i = 0; i < vertices_size; ++i)
	{
		float d = intersect(ray, Sphere(vertices[i], 0.1, vec3(0.0)));
		if (d != 0.0 && d < dist)
		{
			dist = d;
			col = spheres[i].col;
			normal = (ray.start + ray.dir * d) - vertices[i];
			terminate = false;
		}
	}*/
	// ray traversal
	int to_visit_offset = 0;
	int current_node = 0;
	int nodes_to_visit[32];
	while (true)
	{
		Node node = nodes[current_node];
		if (intersect(ray, node))
		{
			if (node.tri_index > -1) // leaf
			{
				// intersect ray with primitive(s) in leaf node
				float d = intersect(ray, node.tri_index * 3);
				if (d != 0.0 && d < dist)
				{
					dist = d;
					col = vec3(1.0, 0.82, 0.11);// vec3(0.7, 1.0, 0.2);
					normal = cross(vertices[indices[node.tri_index * 3 + 2]] - vertices[indices[node.tri_index * 3]],
						vertices[indices[node.tri_index * 3 + 1]] - vertices[indices[node.tri_index * 3]]);
					normal = normalize(normal);
					col = (0.5 * max(dot(normal, vec3(0.0, 0.0, -1.0)), 0.0) + 0.5) * col;
					terminate = false;
				}
				if (to_visit_offset == 0)
					break;
				current_node = nodes_to_visit[--to_visit_offset];
			}
			else // interior
			{
				// put far node on stack, advance to near node
				if (ray.dir[node.axis] < 0)
				{
					nodes_to_visit[to_visit_offset++] = current_node + 1;
					current_node = node.left;
				}
				else
				{
					nodes_to_visit[to_visit_offset++] = node.left;
					current_node = current_node + 1;
				}
			}
		}
		else
		{
			if (to_visit_offset == 0)
				break;
			current_node = nodes_to_visit[--to_visit_offset];
		}
	}


	float d = intersect(ray, plane);
	if (d > 0 && d < dist)
	{
		dist = d;
		col = plane.col;
		terminate = false;
	}
	vec3 dir = normalize(reflect(ray.dir, normal));
	return Ray(ray.start + ray.dir * dist * 0.999, dir, 1.0 / dir, col * ray.col, terminate);
}

void main()
{
	vec4 ray_start = vec4(0.0, 0.0, 0.0, 1.0);
	vec4 ray_end = vec4((TexCoord.x - 0.5) * (9.0/6.0), TexCoord.y - 0.5, -1.0, 1.0);

	vec3 start = (camera * ray_start).xyz;
	vec3 end = (camera * ray_end).xyz;

	vec3 dir = normalize(end - start);
	Ray ray = Ray(start, dir, 1.0 / dir, vec3(1.0, 1.0, 1.0), false);
	ray.dir = normalize(ray.dir);

	for (uint i = 0; i < 8; ++i)
	{
		ray = traceRay(ray);
		if (ray.terminate)
			break;
	}

	/*vec3 bvh_col = vec3(0.0, 0.0, 0.0);

	Ray new_ray = Ray(start, normalize(end - start), vec3(1.0, 1.0, 1.0), false);
	for (int i = 0; i < num_nodes; ++i)
	{
		if (intersect(new_ray, nodes[i]))
		{
			bvh_col[nodes[i].axis] += 0.02;
		}
	}*/
	
	FragColor = vec4(ray.col, 1.0);
}