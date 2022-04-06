#ifndef SHADER_H
#define SHADER_H

#include <GL/glew.h>
#include <GL/gl.h>

#include <cglm/cglm.h>

uint32_t createProgramVF(const char* vertexSourcePath, const char* fragmentSourcePath);

// Uniforms
void uniform1fv(uint32_t program, const char* uniformName, int count, float* value);
void uniform3fv(uint32_t program, const char* uniformName, int count, float* value);

void uniform1i(uint32_t program, const char* uniformName, int value);
void uniform1ui(uint32_t program, const char* uniformName, uint32_t value);

void uniform1f(uint32_t program, const char* uniformName, float value);
void uniform2f(uint32_t program, const char* uniformName, vec2 value);
void uniform3f(uint32_t program, const char* uniformName, vec3 value);

void uniformMatrix4fv(uint32_t program, const char* uniformName, mat4 value);

#endif 
