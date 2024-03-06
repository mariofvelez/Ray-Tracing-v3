#pragma once

#include <algorithm>
#include <vector>

#include <glm/glm.hpp>

#include "Scene.h"

struct Primitive
{
	glm::vec3 min;
	glm::vec3 max;
	glm::vec3 centroid;
	int index; // index into scene array
};
struct BVHNode
{
	glm::vec3 min;
	glm::vec3 max;
	BVHNode* left;
	BVHNode* right;
	int split_axis;
	int prim_start;
	int prim_count;
	int linear_index;

	BVHNode() : linear_index(0)
	{
		
	}
	void initLeaf(int prim_start, int prim_count, glm::vec3& min, glm::vec3& max)
	{
		this->prim_start = prim_start;
		this->prim_count = prim_count;
		this->min = min;
		this->max = max;
		left = right = nullptr;
	}
	void initInterior(int axis, BVHNode* left, BVHNode* right)
	{
		this->left = left;
		this->right = right;
		min = glm::vec3(
			glm::min(left->min.x, right->min.x),
			glm::min(left->min.y, right->min.y),
			glm::min(left->min.z, right->min.z)
		);
		max = glm::vec3(
			glm::max(left->max.x, right->max.x),
			glm::max(left->max.y, right->max.y),
			glm::max(left->max.z, right->max.z)
		);
		split_axis = axis;
		prim_count = 0;
	}
	~BVHNode()
	{
		if(left != NULL)
			delete(left);
		if(right != NULL)
			delete(right);
	}
};

class BVH
{
	BVHNode* root;
	Scene* scene;

	void computeAABB(Primitive& primitive)
	{
		glm::vec3 min = glm::vec3(99999.9f);
		glm::vec3 max = glm::vec3(-99999.9f);

		for (int j = 0; j < 3; ++j)
		{
			glm::vec3 vertex = scene->mesh.vertices[scene->mesh.indices[primitive.index * 3 + j]];
			min.x = glm::min(min.x, vertex.x);
			min.y = glm::min(min.y, vertex.y);
			min.z = glm::min(min.z, vertex.z);
			max.x = glm::max(max.x, vertex.x);
			max.y = glm::max(max.y, vertex.y);
			max.z = glm::max(max.z, vertex.z);
		}
		primitive.min = min;
		primitive.max = max;
	}

public:
	BVH(Scene* scene) : scene(scene)
	{
		root = NULL;
	}
	void computeBVH()
	{
		std::vector<Primitive> primitives;
		primitives.reserve(scene->mesh.index_size / 3);

		// init primitives
		for (unsigned int i = 0; i < scene->mesh.index_size / 3; ++i)
		{
			primitives.emplace_back();
			primitives[i].index = i;
			computeAABB(primitives[i]);
			primitives[i].centroid = 0.5f * primitives[i].min + 0.5f * primitives[i].max;
		}

		int total_nodes;
		std::vector<int> ordered_prims;
		ordered_prims.reserve(primitives.size());
		root = recursiveBuild(primitives, 0, primitives.size(), &total_nodes, ordered_prims);

		/*for (unsigned int i = 0; i < primitives.size(); ++i)
		{
			std::cout << "prim: " << primitives[i].index << std::endl;
			std::cout << "ordr: " << ordered_prims[i] << std::endl;
		}*/

		int new_indices[100000];
		for (unsigned int i = 0; i < ordered_prims.size(); ++i)
		{
			int index = ordered_prims[i];
			new_indices[i * 3] = scene->mesh.indices[index * 3]; // primitives[index].index * 3;
			new_indices[i * 3 + 1] = scene->mesh.indices[index * 3 + 1];
			new_indices[i * 3 + 2] = scene->mesh.indices[index * 3 + 2];
		}
		for (unsigned int i = 0; i < scene->mesh.index_size; ++i)
		{
			scene->mesh.indices[i] = new_indices[i];
		}
	}
	
	BVHNode* recursiveBuild(std::vector<Primitive>& primitives, int start, int end, int* total_nodes, std::vector<int>& ordered_prims)
	{

		BVHNode* node = new BVHNode();
		(*total_nodes)++;
		
		// compute bounds of all primitives
		glm::vec3 min = glm::vec3(std::numeric_limits<float>::max());
		glm::vec3 max = glm::vec3(std::numeric_limits<float>::min());
		for (int i = start; i < end; ++i)
		{
			min.x = glm::min(min.x, primitives[i].min.x);
			min.y = glm::min(min.y, primitives[i].min.y);
			min.z = glm::min(min.z, primitives[i].min.z);
			max.x = glm::max(max.x, primitives[i].max.x);
			max.y = glm::max(max.y, primitives[i].max.y);
			max.z = glm::max(max.z, primitives[i].max.z);
		}

		scene->nodes[scene->num_nodes] = Node();
		scene->nodes[scene->num_nodes].min = glm::vec4(min.x, min.y, min.z, 0.0f);
		scene->nodes[scene->num_nodes].max = glm::vec4(max.x, max.y, max.z, 0.0f);
		scene->nodes[scene->num_nodes].left = -1;
		scene->nodes[scene->num_nodes].num_tris = -1;
		scene->nodes[scene->num_nodes].tri_index = -1;
		node->linear_index = scene->num_nodes;
		scene->num_nodes++;

		int prim_count = end - start;
		if (prim_count <= 4)
		{
			// create leaf node
			int prim_offset = ordered_prims.size();
			for (int i = start; i < end; ++i)
			{
				int prim_num = primitives[i].index;
				ordered_prims.push_back(prim_num);
			}
			node->initLeaf(prim_offset, prim_count, min, max);
			scene->nodes[node->linear_index].tri_index = prim_offset;// primitives[start].index;
			scene->nodes[node->linear_index].num_tris = prim_count;
			return node;
		}
		else
		{
			// compute bound of centroids
			glm::vec3 min = primitives[start].centroid;
			glm::vec3 max = primitives[start].centroid;
			for (int i = start + 1; i < end; ++i)
			{
				min.x = glm::min(min.x, primitives[i].centroid.x);
				min.y = glm::min(min.y, primitives[i].centroid.y);
				min.z = glm::min(min.z, primitives[i].centroid.z);
				max.x = glm::max(max.x, primitives[i].centroid.x);
				max.y = glm::max(max.y, primitives[i].centroid.y);
				max.z = glm::max(max.z, primitives[i].centroid.z);
			}

			// choose split dimension dim
			float curr_extent = max.x - min.x;
			int dim = 0;
			if (max.y - min.y > curr_extent)
			{
				dim = 1;
				curr_extent = max.y - min.y;
			}
			if (max.z - min.z > curr_extent)
			{
				dim = 2;
			}

			// partition primitives into two sets
			int mid = (start + end) / 2;
			if (min[dim] == max[dim])
			{
				//dlogln("here");
				// create leaf node
				int prim_offset = ordered_prims.size();
				for (int i = start; i < end; ++i)
				{
					int prim_num = primitives[i].index;
					ordered_prims.push_back(prim_num);
				}
				node->initLeaf(prim_offset, prim_count, min, max);
				scene->nodes[node->linear_index].tri_index = prim_offset;// primitives[start].index;
				scene->nodes[node->linear_index].num_tris = prim_count;
				return node;
			}
			else
			{
				// partition primitives into two sets
				// through node's midpoint
				float pmid = (min[dim] + max[dim]) * 0.5f;
				Primitive* mid_ptr = std::partition(&primitives[start], &primitives[end-1]+1,
					[dim, pmid](const Primitive& prim) {
						return prim.centroid[dim] < pmid;
					}
				);
				mid = mid_ptr - &primitives[0];

				// equal primitives
				/*mid = (start + end) / 2;
				std::nth_element(&primitives[start], &primitives[mid], &primitives[end-1]+1,
					[dim](const Primitive& a, const Primitive& b) {
						return a.centroid[dim] < b.centroid[dim];
					}
				);*/

				// build children
				node->initInterior(dim,
					recursiveBuild(primitives, start, mid, total_nodes, ordered_prims),
					recursiveBuild(primitives, mid,   end, total_nodes, ordered_prims));
				scene->nodes[node->linear_index].left = node->left->linear_index;
				scene->nodes[node->linear_index].axis = dim;
			}
		}

		return node;
	}
};