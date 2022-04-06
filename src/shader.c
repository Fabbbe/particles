/* Everything that has to do with shader functionality should be 
 * contained within this file! We do as little as possible raw
 * OpenGL calls in the actual main.c
 */
#include "shader.h"

char* readShaderSource(const char* sourcePath) {
	/* Reads and returns the file content, don't forget to free */
	char* shaderSource;
	FILE* fp;

	fp = fopen(sourcePath, "r");
	if (fp == NULL) // Error reading file
		return NULL;
	
	// This tells the length of file
	fseek(fp, 0, SEEK_END);  
	int sourceLength = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	shaderSource = malloc(sizeof(char) * sourceLength + 1);
	fread(shaderSource, 1, sourceLength, fp);
	fclose(fp);

	shaderSource[sourceLength] = 0x00;

	return shaderSource;
}

uint32_t compileShader(const char* shaderSourcePath, uint32_t shaderType) {
	/* Compiles a shader from a given path to the shaders source code.
	 * shaderType is the type of the shader specified as a GLenum
	 *
	 * TODO: Error checking and abstraction
	 */
	uint32_t shader;
	char* shaderSource;


	shaderSource = readShaderSource(shaderSourcePath);

	shader = glCreateShader(shaderType);
	glShaderSource(shader, 1, &shaderSource, NULL);
	glCompileShader(shader);

	free(shaderSource);

	int success;
	char infolog[512];
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(shader, 512, NULL, infolog);
		fprintf(stderr, "Could not compile shader %s: %s\n", shaderSourcePath, infolog);
	}

	return shader;
}

uint32_t createProgramVF(const char* vertexSourcePath, const char* fragmentSourcePath) {
	/* Creates and returns the id of a OpenGL program.
	 *
	 * Takes the path to a vertex and fragment shader and returns an ID for 
	 * an OpenGL program.
	 *
	 */
	uint32_t programVF;
	uint32_t vertexShader;
	uint32_t fragmentShader;

	// Vertex Shader
	vertexShader = compileShader(vertexSourcePath, GL_VERTEX_SHADER);

	// Fragment Shader
	fragmentShader = compileShader(fragmentSourcePath, GL_FRAGMENT_SHADER);

	// Program
	programVF = glCreateProgram();
	glAttachShader(programVF, vertexShader);
	glAttachShader(programVF, fragmentShader);
	glLinkProgram(programVF);

	int success;
	char infolog[512];
	glGetProgramiv(programVF, GL_LINK_STATUS, &success);
	if(!success) {
		fprintf(stderr, "Could not link program\n");
		glGetProgramInfoLog(programVF, 512, NULL, infolog);
	}

	return programVF;
}

/**
 * uniform;
 * @program: The shader program to set the uniform in.
 * @uniformName: The name of the uniform to set.
 * @value: The value to set the uniform to.
 *
 * There might also be a count in any uniform**v functions.
 */
void uniform1fv(uint32_t program, const char* uniformName, int count, float* value) {
	glUseProgram(program);
	uint32_t location = glGetUniformLocation(program, uniformName);
	glUniform1fv(location, count, value);
	glUseProgram(0);
}

void uniform3fv(uint32_t program, const char* uniformName, int count, float* value) {
	glUseProgram(program);
	uint32_t location = glGetUniformLocation(program, uniformName);
	glUniform3fv(location, count, value);
	glUseProgram(0);
}

void uniform1i(uint32_t program, const char* uniformName, int value) {
	glUseProgram(program);
	uint32_t location = glGetUniformLocation(program, uniformName);
	glUniform1i(location, value);
	glUseProgram(0);
}
void uniform1ui(uint32_t program, const char* uniformName, uint32_t value) {
	glUseProgram(program);
	uint32_t location = glGetUniformLocation(program, uniformName);
	glUniform1ui(location, value);
	glUseProgram(0);
}

void uniform1f(uint32_t program, const char* uniformName, float value) {
	glUseProgram(program);
	uint32_t location = glGetUniformLocation(program, uniformName);
	glUniform1f(location, value);
	glUseProgram(0);
}

void uniform2f(uint32_t program, const char* uniformName, vec2 value) {
	glUseProgram(program);
	uint32_t location = glGetUniformLocation(program, uniformName);
	glUniform2f(location, value[0], value[1]);
	glUseProgram(0);
}

void uniform3f(uint32_t program, const char* uniformName, vec3 value) {
	glUseProgram(program);
	uint32_t location = glGetUniformLocation(program, uniformName);
	glUniform3f(location, value[0], value[1], value[2]);
	glUseProgram(0);
}

void uniformMatrix4fv(uint32_t program, const char* uniformName, mat4 value) {
	glUseProgram(program);
	uint32_t location = glGetUniformLocation(program, uniformName);
	glUniformMatrix4fv(location, 1, GL_FALSE, (float*)value);
	glUseProgram(0);
}
