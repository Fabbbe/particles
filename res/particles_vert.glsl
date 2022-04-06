#version 330 core

layout(location = 0) in vec3 squareVerts;
layout(location = 1) in vec4 xyzs; // x, y, z and size
layout(location = 2) in vec4 color;

out vec2 UV;
out vec4 particlecolor;

uniform vec3 cameraRight_worldspace;
uniform vec3 cameraUp_worldspace;
uniform mat4 VP; // View * Projection matrices, no model

void main() {
	float particleSize = xyzs.w;
	vec3 particleCenter_worldspace = xyzs.xyz;
	
	// Defines the size of the particle 
	vec3 vertexPosition_worldspace =
		particleCenter_worldspace
		+ cameraRight_worldspace * squareVerts.x * particleSize
		+ cameraUp_worldspace * squareVerts.y * particleSize;

	gl_Position = VP * vec4(vertexPosition_worldspace, 1.0f);
	particlecolor = color;
}
