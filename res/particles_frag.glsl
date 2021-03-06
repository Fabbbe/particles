#version 330 core

in vec2 UV;
in vec4 particlecolor;

out vec4 color;

uniform sampler2D particle_texture;

void main() {
	color = texture2D(particle_texture, UV) * particlecolor;
}
