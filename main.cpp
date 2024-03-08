#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <stdlib.h>
#include <random>

#include "stb_image.h"

#include "Debug.h"
#include "Camera.h"
#include "Shader.h"
#include "Scene.h"
#include "BVH.h"
#include "Material.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

unsigned int textureFromFile(const std::string& filename, unsigned int texture_type = GL_TEXTURE_2D)
{
	stbi_set_flip_vertically_on_load(true);

	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, num_components;
	std::cout << "filename: " << filename << std::endl;
	unsigned char* data = stbi_load(filename.c_str(), &width, &height, &num_components, 0);
	std::cout << "stbi loaded texture: " << textureID << std::endl;
	if (data)
	{
		glBindTexture(texture_type, textureID);
		glTexImage2D(texture_type, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glTexParameteri(texture_type, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(texture_type, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(texture_type, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(texture_type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << filename << std::endl;
		stbi_image_free(data);
	}
	return textureID;
}
unsigned int CubemapFromFile(std::vector<std::string>& faces, const std::string& directory, bool linearize)
{
	stbi_set_flip_vertically_on_load(false);

	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, num_channels;

	for (unsigned int i = 0; i < faces.size(); ++i)
	{
		std::string filename = faces[i];
		if (directory != "")
			filename = directory + '/' + filename;

		unsigned char* data = stbi_load(filename.c_str(), &width, &height, &num_channels, 0);

		if (data)
		{
			GLenum source_format;
			GLenum format;
			if (num_channels == 1)
			{
				source_format = GL_RED;
				format = GL_RED;
			}
			else if (num_channels == 3)
			{
				if (linearize)
					source_format = GL_SRGB;
				else
					source_format = GL_RGB;
				format = GL_RGB;
			}
			else if (num_channels == 4)
			{
				if (linearize)
					source_format = GL_SRGB_ALPHA;
				else
					source_format = GL_RGBA;
				format = GL_RGB;
			}

			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, source_format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap texture failed to load at path: " << filename << std::endl;
			stbi_image_free(data);
		}
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	const int screen_width = 1200;
	const int screen_height = 800;

	GLFWwindow* window = glfwCreateWindow(screen_width, screen_height, "Ray Tracing v3", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	glViewport(0, 0, screen_width, screen_height);

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	dlogln("creating camera");
	Camera* camera = new Camera(glm::vec3(0.0f, -25.0f, 5.0f),
								glm::vec3(0.0f, 1.0f, 0.0f),
								glm::vec3(0.0f, 0.0f, 1.0f));

	dlogln("creating render quad");
	// quad
	float quad_vertices[] = {
		// positions   // texCoords
		-1.0f,  1.0f,  0.0f, 1.0f,
		-1.0f, -1.0f,  0.0f, 0.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,

		-1.0f,  1.0f,  0.0f, 1.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,
		 1.0f,  1.0f,  1.0f, 1.0f
	};

	unsigned int quadVAO;
	glGenVertexArrays(1, &quadVAO);
	glBindVertexArray(quadVAO);

	unsigned int quadVBO;
	glGenBuffers(1, &quadVBO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), &quad_vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);


	/*MaterialLoader mtl_loader;
	std::vector<Material> materials;
	mtl_loader.loadMaterials("Objects/turtle/material.mtl", &materials);

	for (unsigned int i = 0; i < materials.size(); ++i)
	{
		Material& mat = materials[i];
		dlogln("Material:");
		dlogln("  col: " << mat.albedo.r << ", " << mat.albedo.g << ", " << mat.albedo.b);
	}*/


	dlogln("creating shader");
	Shader* shader = new Shader("Shaders/Vertex.shader", "Shaders/AlbedoFragment.shader");
	shader->use();

	unsigned int camera_loc = shader->uniformLoc("camera");

	glActiveTexture(GL_TEXTURE0);
	unsigned int dragon_texture = textureFromFile("Objects/Stanford_Dragon/DefaultMaterial_baseColor.jpg");
	glBindTexture(GL_TEXTURE_2D, dragon_texture);
	shader->setInt("dragon_texture", 0);

	/*std::vector<std::string> faces
	{
		"right.jpg",
		"left.jpg",
		"top.jpg",
		"bottom.jpg",
		"front.jpg",
		"back.jpg"
	};
	unsigned int cubemap_texture = CubemapFromFile(faces, "skybox", false);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap_texture);
	shader->setInt("cubemap_texture", 1);*/

	glActiveTexture(GL_TEXTURE1);
	unsigned int skybox_texture = textureFromFile("cape_hill.jpg");
	glBindTexture(GL_TEXTURE_2D, skybox_texture);
	shader->setInt("skybox_texture", 1);

	const unsigned int num_spheres = 64;
	/*glm::vec3 cols[] = {
		glm::vec3(1.0f, 0.5f, 0.5f),
		glm::vec3(0.5f, 1.0f, 0.5f),
		glm::vec3(0.5f, 0.5f, 1.0f),
		glm::vec3(0.8f, 0.8f, 0.8f)
	};
	glm::vec3 pos[] = {
		glm::vec3(-1.5f, 0.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(1.5f, 0.0f, 0.0f),
		glm::vec3(3.0f, 0.0f, 0.0f)
	};*/

	srand(5);

	dlogln("creating scene");
	Scene* scene = new Scene();

	std::uniform_real_distribution<float> rand(0.0f, 1.0f);
	std::default_random_engine generator;
	
	for (unsigned int i = 0; i < num_spheres; ++i)
	{
		glm::vec3 col = glm::vec3(rand(generator), rand(generator), rand(generator));
		float radius = rand(generator) * 2.0f + 1.0f;
		glm::vec3 pos = glm::vec3(
			(i % 8) * 3.0f,
			(i / 8.0f) * 3.0f,
			rand(generator) * 1.5f - 0.75f
		);
		/*shader->setVec3("spheres[" + std::to_string(i) + "].pos", pos);
		shader->setVec3("spheres[" + std::to_string(i) + "].col", col);
		shader->setFloat("spheres[" + std::to_string(i) + "].radius", radius);*/



		//scene->loadObject("Objects/icosahedron.obj", pos);
	}
	dlogln("loading object");
	//scene->loadObject("Objects/Stanford_Dragon/scene.obj", glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.1f));
	scene->loadObject("Objects/", "quad.obj", glm::vec3(-15.0f, 15.0f, 0.0f), glm::vec3(1.0f));
	scene->loadObject("Objects/", "icosahedron.obj", glm::vec3(4.0f, 4.0f, 3.0f), glm::vec3(2.0f));
	scene->loadObject("Objects/", "icosahedron.obj", glm::vec3(-4.0f, 4.0f, 5.0f), glm::vec3(2.0f));
	scene->loadObject("Objects/", "icosahedron.obj", glm::vec3(0.0f, -4.0f, 2.0f), glm::vec3(2.0f));

	dlogln("Primitives before:");
	scene->printPrimitives();

	for (unsigned int i = 0; i < scene->materials.size(); ++i)
	{
		Material& mat = scene->materials[i];
		dlogln("Material: " << scene->material_names[i]);
		dlogln("  col: " << mat.albedo.r << ", " << mat.albedo.g << ", " << mat.albedo.b);
	}
	
	/*scene->loadObject("Objects/shuttle.obj", glm::vec3(4.0f, 10.0f, 2.0f), glm::vec3(1.0f));
	scene->loadObject("Objects/shuttle.obj", glm::vec3(4.0f, -10.0f, 2.0f), glm::vec3(1.0f));*/
	
	/*scene.loadObject("Objects/icosahedron.obj", glm::vec3(-1.0f, 0.0f, 2.0f));
	scene.loadObject("Objects/icosahedron.obj", glm::vec3(1.0f, 1.0f, 1.0f));*/

	debug_start(glfwGetTime(), 0);

	dlogln("generating BVH");
	BVH* bvh = new BVH(scene);
	bvh->computeBVH();
	delete(bvh);

	dlogln("creating BVH");
	scene->updateBVH(shader);

	dlogln("Primitives after:");
	scene->printPrimitives();

	dlogln("sending tri data to GPU");
	scene->updateBuffer();

	scene->updateMaterialBuffer();

	scene->updatePrimitiveBuffer();

	debug_end(glfwGetTime(), 0);

	dlogln("BVH build time: " << debug_time(0));

	int curr_frame = 1;

	float delta_time = 0.0f;
	float last_frame = 0.0f;

	float delta_time_list[100];
	for (unsigned int i = 0; i < 100; ++i)
		delta_time_list[i] = 0;
	int dt_index = 0;
	float dt_avg = 0;

	while (!glfwWindowShouldClose(window))
	{
		processInput(window);

		float current_frame = (float)glfwGetTime();
		delta_time = current_frame - last_frame;
		last_frame = current_frame;

		debug_start(glfwGetTime(), 0);

		// camera controls
		camera->processInput(window, delta_time);

		camera->updateProjection(9.0f / 6.0f);
		camera->updateView();

		glm::mat4 inverse = glm::inverse(camera->view);
		glUniformMatrix4fv(camera_loc, 1, GL_FALSE, glm::value_ptr(inverse));
		
		//glEnable(GL_DEPTH_TEST);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		shader->use();
		shader->setInt("curr_frame", curr_frame);
		glBindVertexArray(quadVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glfwSwapBuffers(window);
		glfwPollEvents();

		debug_end(glfwGetTime(), 0);

		curr_frame++;

		delta_time_list[dt_index] = debug_time(0);
		dt_index++;
		dt_index = dt_index % 100;
		dt_avg = 0;
		for (unsigned int i = 0; i < 100; ++i)
			dt_avg += delta_time_list[i];
		dt_avg *= 0.01f;

		//dlogln("time avg: " << dt_avg);
	}

	delete(camera);
	delete(shader);
	delete(scene);

	glfwTerminate();
	return 0;
}