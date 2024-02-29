#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <stdlib.h>
#include <random>

#include "Debug.h"
#include "Camera.h"
#include "Shader.h"
#include "Scene.h"
#include "BVH.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	const int screen_width = 900;
	const int screen_height = 600;

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

	dlogln("creating shader");
	Shader* shader = new Shader("Shaders/Vertex.shader", "Shaders/Fragment.shader");
	shader->use();

	unsigned int camera_loc = shader->uniformLoc("camera");

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
	scene->loadObject("Objects/Stanford_Dragon/Stanford_Dragon_PBR.obj", glm::vec3(0.0f, 0.0f, 1.0f));/*
	scene->loadObject("Objects/shuttle.obj", glm::vec3(4.0f, 10.0f, 2.0f));
	scene->loadObject("Objects/shuttle.obj", glm::vec3(4.0f, -10.0f, 2.0f));*/
	
	/*scene.loadObject("Objects/icosahedron.obj", glm::vec3(-1.0f, 0.0f, 2.0f));
	scene.loadObject("Objects/icosahedron.obj", glm::vec3(1.0f, 1.0f, 1.0f));*/

	debug_start(glfwGetTime(), 0);

	dlogln("creating BVH");
	BVH bvh(scene);
	bvh.computeBVH();

	dlogln("sending BVH data to GPU");
	scene->computeBVH(shader);

	dlogln("sending tri data to GPU");
	scene->updateBuffer();

	debug_end(glfwGetTime(), 0);

	dlogln("BVH build time: " << debug_time(0));

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
		glBindVertexArray(quadVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glfwSwapBuffers(window);
		glfwPollEvents();

		debug_end(glfwGetTime(), 0);

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