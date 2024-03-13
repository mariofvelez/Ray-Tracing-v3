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
#include "Renderer.h"
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

	// camera
	Camera* camera = new Camera(glm::vec3(0.0f, -25.0f, 5.0f),
								glm::vec3(0.0f, 1.0f, 0.0f),
								glm::vec3(0.0f, 0.0f, 1.0f));

	// scene
	Scene* scene = new Scene();

	glActiveTexture(GL_TEXTURE1);
	unsigned int skybox = textureFromFile("environment_map.jpg");
	scene->setEnvironmentMap(skybox);

	// renderer
	Renderer* renderer = new Renderer(camera);
	renderer->createBuffers(screen_width, screen_height);
	
	// loading objects
	scene->loadObject("Objects/Stanford_Dragon/", "scene.obj", glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.1f));
	scene->loadObject("Objects/", "quad.obj", glm::vec3(-15.0f, 15.0f, 0.0f), glm::vec3(1.0f));
	//scene->loadObject("Objects/turtle/", "scene.obj", glm::vec3(0.0f), glm::vec3(1.0f));
	scene->loadObject("Objects/", "icosahedron.obj", glm::vec3(7.0f, 7.0f, 3.0f), glm::vec3(2.0f));
	scene->loadObject("Objects/", "icosahedron.obj", glm::vec3(-5.0f, 7.0f, 12.0f), glm::vec3(2.0f));
	scene->loadObject("Objects/", "icosahedron.obj", glm::vec3(0.0f, -7.0f, 8.0f), glm::vec3(2.0f));

	debug_start(glfwGetTime(), 0);

	// creating BVH
	BVH* bvh = new BVH(scene);
	bvh->computeBVH();
	delete(bvh);
	scene->createBVHBuffer();

	// send vertex data to GPU
	scene->createSceneBuffer();

	// send material data to GPU
	scene->createMaterialBuffer();

	// send primitive data to GPU
	scene->createPrimitiveBuffer();

	debug_end(glfwGetTime(), 0);

	dlogln("BVH build time: " << debug_time(0));


	// imgui
	renderer->initImGui(window);

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
		// imgui update
		renderer->updateImGui();
		if (scene->ImGuiDisplayMaterialTree())
		{
			renderer->resetAccumulate();
		}
		renderer->ImGuiDisplayDebugViewRadio();

		processInput(window);
		if (camera->processInput(window, delta_time))
		{
			renderer->resetAccumulate();
		}

		float current_frame = (float)glfwGetTime();
		delta_time = current_frame - last_frame;
		last_frame = current_frame;

		debug_start(glfwGetTime(), 0);

		// render scene
		renderer->render(scene);

		// imgui render
		renderer->renderImGui();

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
	delete(renderer);
	delete(scene);

	glfwTerminate();
	return 0;
}