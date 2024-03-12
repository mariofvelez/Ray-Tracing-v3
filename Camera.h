#pragma once

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"

class Camera
{
public:
	glm::vec3 m_pos;
	glm::vec3 m_front;

	glm::mat4 view;
	glm::mat4 projection;

	float speed;
	float yaw;
	float pitch;

	Camera(glm::vec3 pos, glm::vec3 front, glm::vec3 up) : m_pos(pos), m_front(front), m_up(up), view(1.0f), projection(1.0f), m_FOV(60.0f), speed(10.0f), pitch(0), near(0.1f), far(500.0f)
	{
		yaw = atan2f(m_front.y, m_front.x);
		updateView();
	}
	~Camera() {}

	void updateView()
	{
		view = glm::lookAt(m_pos, m_pos + m_front, m_up);
	}

	void updateProjection(float aspect)
	{
		projection = glm::perspective(glm::radians(m_FOV), aspect, near, far);
	}

	void uniformView(Shader* shader)
	{
		unsigned int view_loc = shader->uniformLoc("view");
		glUniformMatrix4fv(view_loc, 1, GL_FALSE, glm::value_ptr(view));
	}

	void uniformViewPos(Shader* shader)
	{
		shader->setVec3("viewPos", m_pos);
	}

	void uniformProjection(Shader* shader)
	{
		unsigned int proj_loc = shader->uniformLoc("projection");
		glUniformMatrix4fv(proj_loc, 1, GL_FALSE, glm::value_ptr(projection));
	}

	void move(glm::vec3 move)
	{
		m_pos += move;
	}
	void rotatePitch(float radians)
	{
		pitch += radians;
		if (pitch < -glm::half_pi<float>())
			pitch = -glm::half_pi<float>() + 0.0001f;
		if (pitch > glm::half_pi<float>())
			pitch = glm::half_pi<float>() - 0.0001f;
		m_front.x = cosf(yaw) * cosf(pitch);
		m_front.y = sinf(yaw) * cosf(pitch);
		m_front.z = sinf(pitch);
	}
	void rotateYaw(float radians)
	{
		yaw = atan2f(m_front.y, m_front.x);
		yaw += radians;
		m_front.x = cosf(yaw) * cosf(pitch);
		m_front.y = sinf(yaw) * cosf(pitch);
		m_front.z = sinf(pitch);
	}
	void setFacingDirection(glm::vec3 dir)
	{

	}
	
	bool processInput(GLFWwindow* window, float dt)
	{
		bool updated = false;

		float camera_speed = speed * dt;
		if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
			camera_speed *= 3.0f;

		glm::vec3 forward = glm::normalize(glm::vec3(m_front.x, m_front.y, 0.0f)) * camera_speed;
		glm::vec3 side = glm::normalize(glm::cross(forward, m_up)) * camera_speed;

		if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
		{
			rotateYaw(0.25 * camera_speed);
			updated = true;
		}
		if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		{
			rotateYaw(-0.25 * camera_speed);
			updated = true;
		}
		if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
		{
			rotatePitch(0.25 * camera_speed);
			updated = true;
		}
		if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		{
			rotatePitch(-0.25 * camera_speed);
			updated = true;
		}

		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		{
			move(forward);
			updated = true;
		}
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		{
			move(-side);
			updated = true;
		}
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		{
			move(-forward);
			updated = true;
		}
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		{
			move(side);
			updated = true;
		}
		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		{
			move(glm::vec3(0.0f, 0.0f, camera_speed));
			updated = true;
		}
		if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		{
			move(glm::vec3(0.0f, 0.0f, -camera_speed));
			updated = true;
		}

		updateView();

		return updated;
	}

private:
	glm::vec3 m_up;
	float m_FOV;

	float near;
	float far;
};