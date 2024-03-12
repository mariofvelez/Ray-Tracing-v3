#version 430 core

out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D dragon_texture;
uniform sampler2D skybox_texture;
const float pi = 3.14189265;

struct Node
{
	int axis;
	int left;
	int num_tris;
	int tri_index;
	vec3 min;
	vec3 max;
};

layout(std140, binding = 4) uniform renderData
{
	mat4 camera;
	vec3 camera_pos;
	int num_nodes;
	int curr_frame;
};

layout(std430, binding = 3) buffer bvhBuffer
{
	Node nodes[];
};

struct Ray
{
	vec3 start;
	vec3 dir;
	vec3 inv;
	vec3 col;
	bool terminate;
};

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

	return tmax > tmin && tmax > 0.0;
}

void main()
{
	vec4 ray_start = vec4(0.0, 0.0, 0.0, 1.0);
	vec4 ray_end = vec4((TexCoord.x - 0.5) * (3.0 / 2.0), TexCoord.y - 0.5, -1.0, 1.0);

	vec3 start = (camera * ray_start).xyz;
	vec3 end = (camera * ray_end).xyz;

	vec3 dir = normalize(end - start);
	Ray ray;
	ray.start = start;
	ray.dir = dir;
	ray.inv = 1.0 / dir;
	ray.col = vec3(1.0);
	ray.terminate = false;

	vec3 bvh_col = vec3(0.0, 0.0, 0.0);

	for (int i = 0; i < num_nodes; ++i)
	{
		if (intersect(ray, nodes[i]))
		{
			bvh_col[nodes[i].axis] += 0.02;
		}
	}
	FragColor = vec4(bvh_col, 1.0);
}