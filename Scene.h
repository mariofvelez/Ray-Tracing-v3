#pragma once

#include <string>
#include <fstream>
#include <iostream>

#include <glm/glm.hpp>

#include "Material.h"
#include "Shader.h"

struct Mesh
{
	glm::vec4 vertices[100000];
	glm::vec4 normals[100000];
	glm::vec2 texture[100000];
	int indices[100000];
	int vertex_size = 0;
	int normal_size = 0;
	int texture_size = 0;
	int index_size = 0;
};

struct Node
{
	int axis;
	int left;
	int num_tris;
	int tri_index;
	glm::vec4 min;
	glm::vec4 max;
};

struct Sphere
{
	glm::vec3 pos;
	glm::vec3 col;
	float radius;
};

struct Plane
{
	glm::vec3 pos;
	glm::vec3 normal;
	glm::vec3 col;
};

class Scene
{
public:

	unsigned int data_buffer;
	unsigned int bvh_buffer;

	std::vector<Material> materials;

	Mesh mesh;
	int num_nodes;
	Node nodes[40000];

	Scene() : num_nodes(0)
	{
		for (unsigned int i = 0; i < 100; ++i)
		{
			mesh.indices[i] = 0;
			mesh.vertices[i] = glm::vec4(0.0f);
		}
	}

	void loadObject(const std::string& filename, glm::vec3 offset, glm::vec3 scale)
	{
		glm::vec4 position[1000];
		glm::vec4 normal[1000];
		int position_index = 0;
		int normal_index = 0;

		glm::vec4 offs = glm::vec4(offset.x, offset.y, offset.z, 0.0f);
		glm::vec4 sc   = glm::vec4(scale.x, scale.y, scale.z, 1.0f);

		std::string text;
		std::ifstream file(filename);

		int index_offset = mesh.vertex_size;

		while (std::getline(file, text))
		{
			unsigned int token_length = 0;
			std::string tokens[8];

			std::stringstream ss(text);
			while (ss >> tokens[token_length])
			{
				token_length++;
			}

			if (token_length > 0)
			{
				if (tokens[0] == "v")
				{
					// add vertex
					glm::vec4 v = glm::vec4(std::stof(tokens[1]),
						-std::stof(tokens[3]),
						std::stof(tokens[2]), 1.0f);

					position[position_index] = v * sc + offs;
					position_index++;
					//mesh.vertices[mesh.vertex_size] = v * sc + offs;
					//mesh.vertex_size++;

					//std::cout << "v: " << v.x << ", " << v.y << ", " << v.z << std::endl;
				}
				else if (tokens[0] == "vn")
				{
					glm::vec4 v = glm::vec4(std::stof(tokens[1]),
						-std::stof(tokens[3]),
						std::stof(tokens[2]), 1.0f);

					normal[normal_index] = v;
					normal_index++;
					//mesh.normals[mesh.normal_size] = v;
					//mesh.normal_size++;
				}
				else if (tokens[0] == "vt")
				{
					glm::vec2 v = glm::vec2(std::stof(tokens[1]), std::stof(tokens[2]));

					/*mesh.texture[mesh.texture_size] = v;
					mesh.texture_size++;*/
				}
				else if (tokens[0] == "f")
				{
					// add indices
					int i1 = std::stoi(tokens[1].substr(0, tokens[1].find('/'))) - 1;
					int i2 = std::stoi(tokens[2].substr(0, tokens[2].find('/'))) - 1;
					int i3 = std::stoi(tokens[3].substr(0, tokens[3].find('/'))) - 1;

					std::string index_0_info[3], index_1_info[3], index_2_info[3];
					index_0_info[0] = std::stoi(tokens[1].substr(0, tokens[1].find('/'))) - 1;

					mesh.indices[mesh.index_size] = mesh.index_size;// i1 + index_offset;
					mesh.indices[mesh.index_size + 1] = mesh.index_size + 1; //i2 + index_offset;
					mesh.indices[mesh.index_size + 2] = mesh.index_size + 2; //i3 + index_offset;

					mesh.vertices[mesh.vertex_size] = position[i1];
					mesh.vertices[mesh.vertex_size + 1] = position[i2];
					mesh.vertices[mesh.vertex_size + 2] = position[i3];

					mesh.normals[mesh.normal_size] = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
					mesh.normals[mesh.normal_size + 1] = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
					mesh.normals[mesh.normal_size + 2] = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);

					//std::cout << "f: " << i1 << ", " << i2 << ", " << i3 << std::endl;

					//if (token_length == 5) // quad
					//{
					//	int i4 = std::stoi(tokens[4].substr(0, tokens[4].find('/'))) + index_offset;
					//	mesh.indices[mesh.index_size + 3] = i3 - 1;
					//	mesh.indices[mesh.index_size + 4] = i4 - 1;
					//	mesh.indices[mesh.index_size + 5] = i1 - 1;
					//	mesh.index_size += 3;
					//}
					mesh.index_size += 3;
					mesh.vertex_size += 3;
					mesh.normal_size += 3;
				}
			}
		}
		file.close();
	}

	void updateBuffer()
	{
		//std::cout << "mesh size: " << sizeof(mesh) << std::endl;

		std::cout << "vertices: " << mesh.vertex_size << std::endl;
		std::cout << "indices: " << mesh.index_size << " | triangles: " << (mesh.index_size/3) << std::endl;

		glGenBuffers(1, &data_buffer);
		
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, data_buffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(mesh), &mesh, GL_DYNAMIC_READ);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, data_buffer);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}

	void computeAABB(int start, int len, int bvh_node)
	{
		glm::vec3 min = glm::vec3(99999.9f);
		glm::vec3 max = glm::vec3(-99999.9f);

		for (int i = start; i < start + len; ++i)
		{
			for (int j = 0; j < 3; ++j)
			{
				glm::vec3 vertex = mesh.vertices[mesh.indices[i * 3 + j]];
				min.x = glm::min(min.x, vertex.x);
				min.y = glm::min(min.y, vertex.y);
				min.z = glm::min(min.z, vertex.z);
				max.x = glm::max(max.x, vertex.x);
				max.y = glm::max(max.y, vertex.y);
				max.z = glm::max(max.z, vertex.z);
			}
		}
		nodes[bvh_node].min = glm::vec4(min.x, min.y, min.z, 0.0f);
		nodes[bvh_node].max = glm::vec4(max.x, max.y, max.z, 0.0f);
	}

	void computeBVH(Shader* shader)
	{
		/*int node_index = 0;
		for (unsigned int i = 0; i < mesh.index_size / 3; ++i)
		{
			nodes[node_index] = Node();
			nodes[node_index].left = node_index;
			nodes[node_index].right = node_index;
			nodes[node_index].parent = 0;
			nodes[node_index].tri_index = node_index;
			computeAABB(node_index, 1, node_index);
			
			node_index++;
		}

		num_nodes = node_index;*/

		/*for (unsigned int i = 0; i < num_nodes; ++i)
		{
			std::cout << i << ": axis: " << nodes[i].axis << " | left: " << nodes[i].left << " | right: " << nodes[i].right << " | tri: " << nodes[i].tri_index;
			if (nodes[i].tri_index != -1)
				std::cout << " | leaf";
			std::cout << std::endl;
		}*/

		shader->setInt("num_nodes", num_nodes);

		std::cout << "nodes: " << num_nodes << std::endl;
		//std::cout << "size of nodes: " << sizeof(nodes) << std::endl;

		glGenBuffers(1, &bvh_buffer);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, bvh_buffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(nodes), &nodes, GL_DYNAMIC_READ);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, bvh_buffer);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}
};