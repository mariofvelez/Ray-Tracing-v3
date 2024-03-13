#pragma once

#include <string>
#include <fstream>
#include <iostream>

#include <glm/glm.hpp>

#include "Debug.h"
#include "Material.h"
#include "Shader.h"
#include "ImGuiRenderer.h"

// all vertex, normal, and texture data for the scene
struct SceneData
{
	glm::vec4 vertices[100000];
	glm::vec4 normals[100000];
	glm::vec2 texture[100000];
	int vertex_size = 0;
	int normal_size = 0;
	int texture_size = 0;
};

// primitive triangle data
struct Primitive
{
	// indices of each vertex
	unsigned int vertex_a;
	unsigned int vertex_b;
	unsigned int vertex_c;

	// indices of each vertex normal
	unsigned int normal_a;
	unsigned int normal_b;
	unsigned int normal_c;

	// index ID of the material
	unsigned int material;
};

// flattened BVH node
struct Node
{
	int axis;
	int left;
	int prim_count;
	int prim_index;
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
private:
	bool checkLoadedMaterialFile(std::string& filename)
	{
		for (unsigned int i = 0; i < loaded_material_files.size(); ++i)
		{
			if (loaded_material_files[i] == filename)
				return true;
		}
		return false;
	}
	void setCurrentMaterial(std::string& material_name, unsigned int* curr_material)
	{
		*curr_material = 0u;
		for (unsigned int i = 0; i < material_names.size(); ++i)
		{
			if (material_names[i] == material_name)
			{
				*curr_material = i;
				return;
			}
		}
	}
	glm::ivec3 parseFaceIndex(std::string& face_data)
	{
		glm::ivec3 data = glm::ivec3(0, 0, 0); // vertex, texture, normal

		unsigned int slash_count = 0;
		unsigned int slash_indices[2] = { 0, 0 };
		// traverse string for '/' indices
		for (unsigned int i = 0; i < face_data.size(); ++i)
		{
			if (face_data[i] == '/')
			{
				slash_indices[slash_count] = i;
				slash_count++;
				if (slash_count == 2)
					break;
			}
		}

		if (slash_count == 0)
		{
			data[0] = std::stoi(face_data);
		}
		else if (slash_count == 2)
		{
			data[0] = std::stoi(face_data.substr(0, slash_indices[0])) - 1;
			if (slash_indices[1] - slash_indices[0] > 1)
				data[1] = std::stoi(face_data.substr(slash_indices[0] + 1, slash_indices[1])) - 1;
			data[2] = std::stoi(face_data.substr(slash_indices[1] + 1, face_data.size())) - 1;
		}
		return data;
	}
public:

	unsigned int data_buffer;
	unsigned int material_buffer;
	unsigned int primitive_buffer;
	unsigned int bvh_buffer;

	unsigned int sample_buffer;
	unsigned int accumulate_buffer;

	std::vector<std::string> loaded_material_files;
	std::vector<Material> materials;
	std::vector<std::string> material_names;

	SceneData scene_data;

	int num_primitives;
	Primitive primitives[100000];

	int num_nodes;
	Node nodes[40000];

	unsigned int environment_map;

	Scene() : num_nodes(0)
	{
		// creating default material
		materials.emplace_back(Material());
		Material& default_material = materials[0];
		default_material.albedo = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
		material_names.emplace_back("Default_Material");
	}

	void loadObject(const std::string& path, const std::string& filename, glm::vec3 offset, glm::vec3 scale)
	{
		unsigned int curr_material = 0;

		glm::vec4 offs = glm::vec4(offset.x, offset.y, offset.z, 0.0f);
		glm::vec4 sc   = glm::vec4(scale.x, scale.y, scale.z, 1.0f);

		std::string text;
		std::ifstream file(path + filename);

		unsigned int vertex_offset = scene_data.vertex_size;
		unsigned int texture_offset = scene_data.texture_size;
		unsigned int normal_offset = scene_data.normal_size;

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

					scene_data.vertices[scene_data.vertex_size] = v * sc + offs;
					scene_data.vertex_size++;
					//mesh.vertices[mesh.vertex_size] = v * sc + offs;
					//mesh.vertex_size++;

					//std::cout << "v: " << v.x << ", " << v.y << ", " << v.z << std::endl;
				}
				else if (tokens[0] == "vn")
				{
					glm::vec4 v = glm::vec4(std::stof(tokens[1]),
						-std::stof(tokens[3]),
						std::stof(tokens[2]), 1.0f);

					scene_data.normals[scene_data.normal_size] = v;
					scene_data.normal_size++;
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
					glm::ivec3 i1 = parseFaceIndex(tokens[1]);
					glm::ivec3 i2 = parseFaceIndex(tokens[2]);
					glm::ivec3 i3 = parseFaceIndex(tokens[3]);

					primitives[num_primitives] = Primitive();
					Primitive& p = primitives[num_primitives];
					p.vertex_a = i1.x + vertex_offset;
					p.vertex_b = i2.x + vertex_offset;
					p.vertex_c = i3.x + vertex_offset;
					p.normal_a = i1.z + normal_offset;
					p.normal_b = i2.z + normal_offset;
					p.normal_c = i3.z + normal_offset;
					p.material = curr_material;
					num_primitives++;

					if (token_length == 5)
					{
						glm::ivec3 i4 = parseFaceIndex(tokens[4]);

						primitives[num_primitives] = Primitive();
						Primitive& p = primitives[num_primitives];
						p.vertex_a = i3.x + vertex_offset;
						p.vertex_b = i4.x + vertex_offset;
						p.vertex_c = i1.x + vertex_offset;
						p.normal_a = i3.z + normal_offset;
						p.normal_b = i4.z + normal_offset;
						p.normal_c = i1.z + normal_offset;
						p.material = curr_material;
						num_primitives++;
					}

					//// add indices
					//int i1 = std::stoi(tokens[1].substr(0, tokens[1].find('/'))) - 1;
					//int i2 = std::stoi(tokens[2].substr(0, tokens[2].find('/'))) - 1;
					//int i3 = std::stoi(tokens[3].substr(0, tokens[3].find('/'))) - 1;

					//mesh.indices[mesh.index_size] = mesh.index_size;// i1 + index_offset;
					//mesh.indices[mesh.index_size + 1] = mesh.index_size + 1; //i2 + index_offset;
					//mesh.indices[mesh.index_size + 2] = mesh.index_size + 2; //i3 + index_offset;

					//mesh.vertices[mesh.vertex_size] = position[i1];
					//mesh.vertices[mesh.vertex_size + 1] = position[i2];
					//mesh.vertices[mesh.vertex_size + 2] = position[i3];

					//mesh.normals[mesh.normal_size] = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
					//mesh.normals[mesh.normal_size + 1] = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
					//mesh.normals[mesh.normal_size + 2] = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);

					//std::cout << "f: " << i1 << ", " << i2 << ", " << i3 << std::endl;

					//if (token_length == 5) // quad
					//{
					//	int i4 = std::stoi(tokens[4].substr(0, tokens[4].find('/'))) + index_offset;
					//	mesh.indices[mesh.index_size + 3] = i3 - 1;
					//	mesh.indices[mesh.index_size + 4] = i4 - 1;
					//	mesh.indices[mesh.index_size + 5] = i1 - 1;
					//	mesh.index_size += 3;
					//}
					/*mesh.index_size += 3;
					mesh.vertex_size += 3;
					mesh.normal_size += 3;*/
				}
				else if (tokens[0] == "mtlib" || tokens[0] == "mtllib")
				{
					// check if mtl file was already loaded
					if (!checkLoadedMaterialFile(tokens[1]))
					{
						loaded_material_files.push_back(tokens[1]);

						// load mtl file
						MaterialLoader mat_loader;
						mat_loader.loadMaterials(path + tokens[1], &materials, &material_names);
					}
				}
				else if (tokens[0] == "usemtl")
				{
					// search for material and set current material
					setCurrentMaterial(tokens[1], &curr_material);
				}
			}
		}
		file.close();
	}

	void createSceneBuffer()
	{
		//std::cout << "mesh size: " << sizeof(mesh) << std::endl;
		dlogln("vertices: " << scene_data.vertex_size << " | normals: " << scene_data.normal_size << " | primitives: " << num_primitives);
		dlogln("BVH nodes: " << num_nodes);

		glGenBuffers(1, &data_buffer);
		
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, data_buffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(scene_data), &scene_data, GL_DYNAMIC_READ);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, data_buffer);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}

	void createMaterialBuffer()
	{
		glGenBuffers(1, &material_buffer);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, material_buffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Material) * materials.size(), &materials[0], GL_DYNAMIC_READ);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, material_buffer);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}

	void createPrimitiveBuffer()
	{
		glGenBuffers(1, &primitive_buffer);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, primitive_buffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(primitives), &primitives, GL_DYNAMIC_READ);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, primitive_buffer);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}

	void createBVHBuffer()
	{
		std::cout << "nodes: " << num_nodes << std::endl;

		glGenBuffers(1, &bvh_buffer);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, bvh_buffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(nodes), &nodes, GL_DYNAMIC_READ);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, bvh_buffer);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}

	void updateMaterialBuffer()
	{
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, material_buffer);
		GLvoid* p = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY);
		memcpy(p, &materials[0], sizeof(Material) * materials.size());
		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	}

	void setEnvironmentMap(unsigned int map)
	{
		environment_map = map;
	}

	bool ImGuiDisplayMaterialTree()
	{
		bool updated = false;

		static ImGuiTableFlags flags = ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersOuterH | ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg | ImGuiTableFlags_NoBordersInBody;
		static ImGuiTreeNodeFlags tree_node_flags = ImGuiTreeNodeFlags_SpanAllColumns;

		ImGui::BeginTable("Materials", 1, flags);
		ImGui::TableSetupColumn("Materials", ImGuiTableColumnFlags_NoHide);
		ImGui::TableHeadersRow();

		for (unsigned int i = 0; i < materials.size(); ++i)
		{
			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			Material& mat = materials[i];
			bool open = ImGui::TreeNodeEx(material_names[i].c_str(), tree_node_flags);

			if (open)
			{
				// display properties
				if (ImGui::ColorEdit3("Albedo", (float*)&mat.albedo, ImGuiColorEditFlags_Float))
				{
					updateMaterialBuffer();
					updated = true;
				}
				if (ImGui::SliderFloat("Roughness", &mat.roughness, 0.0f, 1.0f))
				{
					updateMaterialBuffer();
					updated = true;
				}
				if (ImGui::SliderFloat("Metallic", &mat.metal, 0.0f, 1.0f))
				{
					updateMaterialBuffer();
					updated = true;
				}
				if (ImGui::SliderFloat("Emission", &mat.emission, 1.0f, 50.0f, "%.3f", ImGuiSliderFlags_Logarithmic))
				{
					updateMaterialBuffer();
					updated = true;
				}
				ImGui::TreePop();
			}
		}

		ImGui::EndTable();

		return updated;
	}

	void printPrimitives()
	{
		for (unsigned int i = 0; i < num_primitives; ++i)
		{
			dlogln(i << ": " << primitives[i].vertex_a << ", " << primitives[i].vertex_b << ", " << primitives[i].vertex_c << " | mat: " << material_names[primitives[i].material]);
		}
	}

	void printNodes()
	{
		for (unsigned int i = 0; i < num_nodes; ++i)
		{
			std::cout << i << ": axis: " << nodes[i].axis << " | left: " << nodes[i].left << " | prim: " << nodes[i].prim_index;
			if (nodes[i].prim_count != -1)
				std::cout << " | leaf";
			std::cout << std::endl;
		}
	}
};