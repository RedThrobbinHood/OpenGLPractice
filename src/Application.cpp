#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include "Renderer.h"

#include "VertexBuffer.h"
#include "IndexBuffer.h"


struct ShaderProgramSource
{
	std::string VertexSource;
	std::string FragmentSource;
};

static ShaderProgramSource ParseShader(const std::string& filepath)
{
	std::ifstream stream(filepath);

	enum class ShaderType
	{
		NONE = -1, VERTEX = 0, FRAGMENT = 1
	};

	std::string line;
	std::stringstream ss[2];
	ShaderType type = ShaderType::NONE;
	while (getline(stream, line))
	{
		if (line.find("#shader") != std::string::npos)
		{
			if (line.find("vertex") != std::string::npos)
			{
				type = ShaderType::VERTEX;
			}
			else if (line.find("fragment") != std::string::npos)
			{
				type = ShaderType::FRAGMENT;
			}
		}
		else
		{
			ss[(int)type] << line << '\n';
		}
	}

	return { ss[0].str(), ss[1].str() };
}

static unsigned int CompileShader(unsigned int type, const std::string& source)
{
	unsigned int id = glCreateShader(type);
	const char* src = source.c_str();
	glShaderSource(id, 1, &src, nullptr);
	glCompileShader(id);

	// Check if shader failed to compile
	int result;
	glGetShaderiv(id, GL_COMPILE_STATUS, &result);
	if (result == GL_FALSE)
	{
		int length;
		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
		char* message = (char*)alloca(length * sizeof(char));
		glGetShaderInfoLog(id, length, &length, message);
		std::cout << "Failed to compile " << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << " shader!" << std::endl;
		std::cout << message << std::endl;
		glDeleteShader(id);
		return 0;
	}

	return id;
}

// Reads shader data and links it, returns a unique identifier (int) so we can use it
static unsigned int CreateShader(const std::string& vertexShader, const std::string& fragmentShader)
{
	// Create program to attach shader objects to
	unsigned int program = glCreateProgram();

	// Create shaders
	unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
	unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

	// Attach and shaders and link program
	glAttachShader(program, vs);
	glAttachShader(program, fs);
	glLinkProgram(program);

	// Validate program
	glValidateProgram(program);

	// Delete shader data
	glDeleteShader(vs);
	glDeleteShader(fs);

	// Return program
	return program;
}

int main(void)
{
	GLFWwindow* window;
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	/* Initialize the library */
	if (!glfwInit())
		return -1;


	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(640, 480, "GL Test", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	/* Make the window's context current */
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	if (glewInit() != GLEW_OK)
	{
		std::cout << "ERROR!" << std::endl;
	}

	{
		// Store the X/Y vectors of 3 verticies for triangle we want to draw
		float triVertPositions[] = {
			-0.5f, -0.5f, // 0
			0.5f, -0.5f, // 1
			0.5f, 0.5f, // 2
			-0.5f, 0.5f, // 3
		};

		unsigned int indices[] = {
			0, 1, 2,
			2, 3, 0
		};

		unsigned int vao;
		GLCall(glGenVertexArrays(1, &vao));
		GLCall(glBindVertexArray(vao));

		VertexBuffer vb(triVertPositions, 4 * 2 * sizeof(float));

		//Enable attrib pointers
		GLCall(glEnableVertexAttribArray(0));
		GLCall(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, 0));

		IndexBuffer ib(indices, 6);

		ShaderProgramSource source = ParseShader("res/shaders/Basic.shader");
		unsigned int shader = CreateShader(source.VertexSource, source.FragmentSource);
		GLCall(glUseProgram(shader));

		GLCall(int location = glGetUniformLocation(shader, "u_Color"));
		ASSERT(location != -1);
		GLCall(glUniform4f(location, 0.2f, 0.3f, 0.8f, 1.0f));

		float r = 0.0f;
		float increment = 0.05f;

		/* Loop until the user closes the window */
		while (!glfwWindowShouldClose(window))
		{
			/* Render here */
			GLCall(glClear(GL_COLOR_BUFFER_BIT));

			GLCall(glUseProgram(shader));
			GLCall(glUniform4f(location, r, 0.3f, 0.8f, 1.0f));

			GLCall(glBindVertexArray(vao));
			ib.Bind();


			// (We are drawing a trangles type, we are starting at element 0 in the given array, we are drawing 3 indicies (in this case verticies))
			GLCall(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr));

			if (r > 1.0f)
				increment = -0.05f;
			else if (r < 0.0f)
				increment = 0.05f;

			r += increment;

			/* Swap front and back buffers */
			GLCall(glfwSwapBuffers(window));

			/* Poll for and process events */
			GLCall(glfwPollEvents());
		}

		GLCall(glDeleteProgram(shader));
	}
	glfwTerminate();
	return 0;
}