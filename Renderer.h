#pragma once

#include "Shader.h"
#include "Camera.h"

class Renderer
{
	unsigned int sample_buffer;
	unsigned int accumulate_buffer;
	unsigned int result_buffer;

	unsigned int sample_texture;
	unsigned int accumulate_texture;
	unsigned int result_texture;

	Shader* accumulate_shader; // combines sample and accumulate textures
	Shader* post_process_shader; // renders accumulate texture to default buffer with post processing

	int current_frame;

	Camera* camera;

	void sampleRender(Shader* shader)
	{
		// render scene
	}

	void accumulateRender()
	{
		// combine accumulate and sample buffer into result buffer
	}

	void copyResultIntoAccumulate()
	{
		// update accumulate buffer
	}

public:
	Renderer() : current_frame(1)
	{

	}

	void createBuffers(int screen_width, int screen_height)
	{
		glActiveTexture(GL_TEXTURE0);

		// creating sample buffer
		glGenFramebuffers(1, &sample_buffer);
		glBindFramebuffer(GL_FRAMEBUFFER, sample_buffer);

		glGenTextures(1, &sample_texture);
		glBindTexture(GL_TEXTURE_2D, sample_texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screen_width, screen_height, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, sample_texture, 0);

		// creating accumulate buffer
		glGenFramebuffers(1, &accumulate_buffer);
		glBindFramebuffer(GL_FRAMEBUFFER, accumulate_buffer);

		glGenTextures(1, &accumulate_texture);
		glBindTexture(GL_TEXTURE_2D, accumulate_texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screen_width, screen_height, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, accumulate_texture, 0);

		// creating result buffer
		glGenFramebuffers(1, &result_buffer);
		glBindFramebuffer(GL_FRAMEBUFFER, result_buffer);

		glGenTextures(1, &result_texture);
		glBindTexture(GL_TEXTURE_2D, result_texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screen_width, screen_height, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, result_texture, 0);
	}

	void render(Shader* shader)
	{
		sampleRender(shader);

		accumulateRender();

		copyResultIntoAccumulate();

		current_frame++;
	}

	void resetAccumulate()
	{
		current_frame = 1;
	}
};