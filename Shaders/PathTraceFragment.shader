#version 430 core

out vec4 FragColor;

in vec2 TexCoord;

//uniform Sphere spheres[100];
//const Plane plane = Plane(vec3(0.0, 0.0, 0.0), vec3(0.0, 0.0, 1.0), vec3(0.5, 0.5, 0.5));
uniform sampler2D dragon_texture;
uniform sampler2D skybox_texture;
const float pi = 3.14189265;

struct Material
{
	vec3 albedo;
	//float roughness;

	//vec3 emission;
	//vec3 emission_power;
};

struct Primitive
{
	uint vertex_a;
	uint vertex_b;
	uint vertex_c;

	uint normal_a;
	uint normal_b;
	uint normal_c;

	uint material;
};

struct Node
{
	int axis;
	int left;
	int prim_count;
	int prim_index;
	vec3 min;
	vec3 max;
};

struct Ray
{
	vec3 start;
	vec3 dir;
	vec3 inv;
	vec3 col;
	bool terminate;
};

layout(std140, binding = 4) uniform renderData
{
	mat4 camera;
	vec3 camera_pos;
	int num_nodes;
	int curr_frame;
};

layout(std430, binding = 0) buffer sceneBuffer
{
	vec3 vertices[100000];
	vec3 normals[100000];
	vec2 textures[100000];
	int vertices_size;
	int normals_size;
	int textures_size;
};

layout(std430, binding = 1) buffer materialBuffer
{
	Material materials[];
};

layout(std430, binding = 2) buffer primitiveBuffer
{
	Primitive primitives[];
};

layout(std430, binding = 3) buffer bvhBuffer
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

//struct Sphere
//{
//	vec3 pos;
//	float radius;
//	vec3 col;
//};
//
//struct Plane
//{
//	vec3 pos;
//	vec3 normal;
//	vec3 col;
//};

uint pcg_hash(uint input)
{
	uint state = input * 747796705u + 2891336453u;
	uint word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
	return (word >> 22u) ^ word;
}

float rand_float(inout uint seed)
{
	seed = pcg_hash(seed);
	return float(seed) / float(0xffffffffu);
}

vec3 on_unit_hemisphere(vec3 normal, inout uint seed)
{
	float x = rand_float(seed);
	float y = rand_float(seed);
	float z = rand_float(seed);
	vec3 vec = normalize(vec3(x * 2.0 - 1.0, y * 2.0 - 1.0, z * 2.0 - 1.0));
	if (dot(vec, -normal) < 0.0)
		vec *= -1.0;
	return vec;
}

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
//float intersect(Ray ray, Plane plane)
//{
//	float t = dot(plane.normal, plane.pos - ray.start) / dot(plane.normal, ray.dir);
//	if (t > 0.00001)
//		return t;
//	return 0.0;
//}

vec3 intersect(Ray ray, Primitive prim)
{
	vec3 a = vertices[prim.vertex_a];
	vec3 b = vertices[prim.vertex_b];
	vec3 c = vertices[prim.vertex_c];

	vec3 e1 = b - a;
	vec3 e2 = c - a;

	vec3 ray_cross_e2 = cross(ray.dir, e2);

	float det = dot(e1, ray_cross_e2);

	if (det > -0.0000001 && det < 0.0000001)
		return vec3(0.0);

	float inv_det = 1.0 / det;
	vec3 s = ray.start - a;

	float u = inv_det * dot(s, ray_cross_e2);

	if (u < 0 || u > 1)
		return vec3(0.0);

	vec3 s_cross_e1 = cross(s, e1);

	float v = inv_det * dot(ray.dir, s_cross_e1);

	if (v < 0 || u + v > 1)
		return vec3(0.0);

	float t = inv_det * dot(e2, s_cross_e1);

	return t > 0.00001 ? vec3(t, u, v) : vec3(0.0);
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

	return tmax > tmin && tmax > 0.0;
}

Ray traceRay(Ray ray, inout uint seed)
{
	float dist = 999999.9;
	vec2 tex_coord = vec2((atan(ray.dir.y, ray.dir.x) + pi/2.0) / (pi*2.0), (asin(ray.dir.z) + pi/2.0) / pi);
	vec3 col = texture(skybox_texture, tex_coord).xyz;// vec3(1.0);// vec3(0.4, 0.8, 1.0);
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
			if (node.prim_index > -1) // leaf
			{
				// intersect ray with primitive(s) in leaf node
				for (int i = 0; i < node.prim_count; ++i)
				{
					Primitive prim = primitives[node.prim_index + i];
					vec3 intersection = intersect(ray, prim);
					if (intersection.x > 0.0 && intersection.x < dist)
					{
						dist = intersection.x;
						float u = intersection.y;
						float v = intersection.z;
						/*vec2 a = textures[indices[tri]];
						vec2 b = textures[indices[tri + 1]];
						vec2 c = textures[indices[tri + 2]];*/
						col = materials[prim.material].albedo; //texture(dragon_texture, (1 - u - v) * a + u * b + v * c).xyz; vec3(0.7, 1.0, 0.2); vec3(1 - intersection.y - intersection.z, intersection.yz);
						/*normal = cross(vertices[indices[tri + 2]] - vertices[indices[tri]],
							vertices[indices[tri + 1]] - vertices[indices[tri]]);*/
						normal = (1 - u - v) * normals[prim.normal_a] + (u * normals[prim.normal_b]) + (v * normals[prim.normal_c]);
						normal = -normalize(normal);
						//col = (0.8 * max(dot(normal, vec3(0.0, 0.0, -1.0)), 0.0) + 0.2) * col;
						terminate = false;
					}
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


	/*float d = intersect(ray, plane);
	if (d > 0 && d < dist)
	{
		dist = d;
		col = plane.col;
		terminate = false;
	}*/
	vec3 offs = vec3(rand_float(seed), rand_float(seed), rand_float(seed)) * 0.1;
	vec3 dir = normalize(reflect(ray.dir, normal) + offs); // on_unit_hemisphere(normal, seed);
	if (rand_float(seed) < 0.9)
		dir = on_unit_hemisphere(normal, seed);
	return Ray(ray.start + ray.dir * dist * 0.999, dir, 1.0 / dir, ray.col * col, terminate);
}

void main()
{
	ivec2 screen_coord = ivec2(int((TexCoord.x + 1.0) * 600), int((TexCoord.y + 1.0) * 400));
	uint seed = uint(screen_coord.y * 1200 + screen_coord.x + curr_frame * 1200 * 800);

	int num_samples = 1;
	for (uint j = 0; j < num_samples; ++j)
	{
		vec4 ray_start = vec4(0.0, 0.0, 0.0, 1.0);
		vec4 ray_end = vec4((TexCoord.x - 0.5) * (3.0 / 2.0) + rand_float(seed) / 1200.0, TexCoord.y - 0.5 + rand_float(seed) / 800.0, -1.0, 1.0);

		vec3 start = (camera * ray_start).xyz;
		vec3 end = (camera * ray_end).xyz;

		vec3 dir = normalize(end - start);
		Ray ray; //= Ray(start, dir, 1.0 / dir, vec3(1.0, 1.0, 1.0), false);
		ray.start = start;
		ray.dir = dir;
		ray.inv = 1.0 / dir;
		ray.col = vec3(1.0);
		ray.terminate = false;

		for (uint i = 0; i < 4; ++i)
		{
			ray = traceRay(ray, seed);
			if (ray.terminate)
				break;
		}

		if (!ray.terminate)
			ray.col = vec3(0.0);

		/*vec3 bvh_col = vec3(0.0, 0.0, 0.0);

		Ray new_ray = Ray(start, normalize(end - start), 1.0 / normalize(end - start), vec3(1.0, 1.0, 1.0), false);
		for (int i = 0; i < num_nodes; ++i)
		{
			if (intersect(new_ray, nodes[i]))
			{
				bvh_col[nodes[i].axis] += 0.02;
			}
		}*/
		
		FragColor += vec4(ray.col, 1.0);
	}
	FragColor /= float(num_samples);
}