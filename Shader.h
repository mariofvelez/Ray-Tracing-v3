#pragma once
#include <glad/glad.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include <glm/glm.hpp>

class Shader
{
public:
	unsigned int m_ID;

	Shader(const char* vertex_path, const char* fragment_path)
	{
		// retrieve code from path
		std::string vertex_code;
		std::string fragment_code;

		std::ifstream v_shader_file;
		std::ifstream f_shader_file;

		v_shader_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		f_shader_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		try
		{
			v_shader_file.open(vertex_path);
			f_shader_file.open(fragment_path);
			std::stringstream v_shader_stream, f_shader_stream;

			v_shader_stream << v_shader_file.rdbuf();
			f_shader_stream << f_shader_file.rdbuf();

			v_shader_file.close();
			f_shader_file.close();

			vertex_code = v_shader_stream.str();
			fragment_code = f_shader_stream.str();
		}
		catch (std::ifstream::failure e)
		{
			std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ" << std::endl;
		}
		const char* v_shader_code = vertex_code.c_str();
		const char* f_shader_code = fragment_code.c_str();

		// compile shaders
		unsigned int vertex, fragment;
		int success;
		char infolog[512];

		vertex = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex, 1, &v_shader_code, NULL);
		glCompileShader(vertex);

		glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(vertex, 512, NULL, infolog);
			std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infolog << std::endl;
		}

		fragment = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment, 1, &f_shader_code, NULL);
		glCompileShader(fragment);

		glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(fragment, 512, NULL, infolog);
			std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infolog << std::endl;
		}

		m_ID = glCreateProgram();
		glAttachShader(m_ID, vertex);
		glAttachShader(m_ID, fragment);
		glLinkProgram(m_ID);

		glGetProgramiv(m_ID, GL_LINK_STATUS, &success);
		if (!success)
		{
			glGetProgramInfoLog(m_ID, 512, NULL, infolog);
			std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infolog << std::endl;
		}

		glDeleteShader(vertex);
		glDeleteShader(fragment);
	}

	void addGeometryShader(const char* path)
	{
		// finish implementation
	}

	void use()
	{
		glUseProgram(m_ID);
	}

	unsigned int uniformLoc(const char* name)
	{
		return glGetUniformLocation(m_ID, name);
	}
	unsigned int uniformLoc(std::string& name)
	{
		return glGetUniformLocation(m_ID, name.c_str());
	}
	void setInt(const std::string& name, int val) const
	{
		glUniform1i(glGetUniformLocation(m_ID, name.c_str()), val);
	}
	void setFloat(const std::string& name, float val) const
	{
		glUniform1f(glGetUniformLocation(m_ID, name.c_str()), val);
	}
	void setVec2(const std::string& name, glm::vec2& vec) const
	{
		glUniform2f(glGetUniformLocation(m_ID, name.c_str()), vec.x, vec.y);
	}
	void setVec2(const std::string& name, const glm::vec2& vec) const
	{
		glUniform2f(glGetUniformLocation(m_ID, name.c_str()), vec.x, vec.y);
	}
	void setVec2(const std::string& name, float x, float y) const
	{
		glUniform2f(glGetUniformLocation(m_ID, name.c_str()), x, y);
	}
	void setVec3(const std::string& name, glm::vec3& vec) const
	{
		glUniform3f(glGetUniformLocation(m_ID, name.c_str()), vec.x, vec.y, vec.z);
	}
	void setVec3(const std::string& name, float x, float y, float z) const
	{
		glUniform3f(glGetUniformLocation(m_ID, name.c_str()), x, y, z);
	}
};