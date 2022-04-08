/* Particles
 * Fabian Beskow (c) 2022
 *
 * A small program to render particles in an efficient way through instancing. 
 * The particles start at a random position between -0.5 to 0.5 on all axees 
 * and with a random speed.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>

#include <GL/glew.h>
#include <GL/gl.h>

#include <SDL2/SDL.h>

#include <cglm/cglm.h>

#include "shader.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#ifndef NDEBUG
void debugCallback(GLenum source, GLenum type, GLuint id,
   GLenum severity, GLsizei length, const GLchar* message, const void* userParam);
#endif

float rand_float();

// A quad to be rendered as particle of length 12
static const float g_vertex_buffer_data[] = {
	-0.5f, -0.5f, 0.0f,
	 0.5f, -0.5f, 0.0f,
	-0.5f,  0.5f, 0.0f,
	 0.5f,  0.5f, 0.0f,
};

struct Particle {
	vec3 pos, speed;
	unsigned char r,g,b,a;
	float size;
};
// This can be increased to about 100 000 with about 60% cpu usage
const int max_particles = 50000; 
const float particle_accel = 0.00001f;
const float particle_init_speed = 0.07f;
const float particle_size = 0.025f;
const char particle_color[4] = {255, 60, 60, 170}; // r g b a

const float fov = 0.7f;
const float movespeed = 0.01f;

int main(int argc, char* argv[]) {
	SDL_Window* window = NULL;
	SDL_GLContext context;

	int window_width = 640;
	int window_height = 480;

	srand(time(NULL));

	freopen("error.log", "w", stderr);

	// INITIALIZATION
	// ==============
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0) {
		fprintf(stderr, "Failed to init SDL: %s\n", SDL_GetError());
	}

	window = SDL_CreateWindow(
		"Particles",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		window_width, window_height,
		SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
	);

	if (window == NULL) {
		fprintf(stderr, "Failed to create window: %s\n", SDL_GetError());
	}

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);

	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1); 
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

	context = SDL_GL_CreateContext(window);
	if (context == NULL) {
		fprintf(stderr, "Could not create OpenGL context: %s\n", SDL_GetError());
	}

	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Could not init GLEW\n");
	}

#ifndef NDEBUG
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(debugCallback, NULL); 
#endif

	glEnable(GL_MULTISAMPLE);
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Gives alpha to particles

	// GL setup
	// --------
	
	uint32_t program = createProgramVF("res/particles_vert.glsl","res/particles_frag.glsl");

	uint32_t vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	uint32_t particle_vertex_buffer;
	uint32_t particle_position_buffer;
	uint32_t particle_color_buffer;

	glGenBuffers(1, &particle_vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, particle_vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

	glGenBuffers(1, &particle_position_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, particle_position_buffer);
	glBufferData(GL_ARRAY_BUFFER, max_particles*4*sizeof(float), NULL, GL_STATIC_DRAW);
	

	glGenBuffers(1, &particle_color_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, particle_color_buffer);
	glBufferData(GL_ARRAY_BUFFER, max_particles*4*sizeof(unsigned char), NULL, GL_STATIC_DRAW);

	// Image
	
	uint32_t tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	int width, height, comp;
	stbi_set_flip_vertically_on_load(true);
	unsigned char* particle_image = stbi_load("res/particle.png", &width, &height, &comp, 0);

	glActiveTexture(GL_TEXTURE0);
	glTexImage2D(
		GL_TEXTURE_2D, 
		0,
		GL_RGBA,
		width, height,
		0,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		particle_image
	);

	// Particle data
	struct Particle* particle_container  = malloc(sizeof(struct Particle)*max_particles);
	float* g_particle_position_size_data = malloc(sizeof(float)*4*max_particles);
	unsigned char* g_particle_color_data = malloc(sizeof(char)*4*max_particles);

	for (int i = 0; i < max_particles; ++i) {
		struct Particle* pp = &particle_container[i];
		pp->pos[0] = rand_float()-0.5f;
		pp->pos[1] = rand_float()-0.5f;
		pp->pos[2] = rand_float()-0.5f;
		pp->size = particle_size;
		pp->speed[0] = (rand_float()-0.5f)*particle_init_speed;
		pp->speed[1] = (rand_float()-0.5f)*particle_init_speed;
		pp->speed[2] = (rand_float()-0.5f)*particle_init_speed;

		pp->r = particle_color[0];
		pp->g = particle_color[1];
		pp->b = particle_color[2];
		pp->a = particle_color[3];
	}

	mat4 view = GLM_MAT4_IDENTITY_INIT;
	mat4 proj = GLM_MAT4_IDENTITY_INIT;
	mat4 vp;

	vec3 camera_pos = {0.0f, 0.0f, -10.0f};
	vec3 camera_dir = {0.0f, 0.0f, 1.0f};

	glm_look(camera_pos, camera_dir, GLM_YUP, view);
	glm_perspective(fov, (float)window_width/(float)window_height, 0.001f, 1000.0f, proj);

	uint64_t last_t;
	uint64_t now_t = SDL_GetPerformanceCounter();
	double delta_t = 1.0f;

	// RUNNING
	// =======
	bool physics = true;
	bool running = true;
	while (running) {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_QUIT:
					running = false;
					break;

				case SDL_KEYDOWN:
					switch (event.key.keysym.sym) { // Run through all the keys
						case SDLK_F4:
							if (SDL_GetRelativeMouseMode())
								SDL_SetRelativeMouseMode(SDL_FALSE);
							else
								SDL_SetRelativeMouseMode(SDL_TRUE);
							break;
						case SDLK_SPACE:
							physics = physics ? false : true;
					}
					break;

				case SDL_WINDOWEVENT:
					if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
						SDL_GetWindowSize(window, &window_width, &window_height);

						glm_perspective(
							fov, 
							(float)window_width/(float)window_height, 
							0.001f, 
							1000.0f, 
							proj
						);

						glViewport(0, 0, window_width, window_height);
					}
					break;
			}
		}

		// User input!
		// -----------
		
		vec3 move_dir; // will be the normalized vector to move relative to camera
		glm_vec3_zero(move_dir);
		{	// Keyboard state reading and input
			// This is a nice way of getting game-like input
			const unsigned char* keyboardState = SDL_GetKeyboardState(NULL); 

			// flying camera controls
			// I see a lot of people write this by watching for presses/releases 
			// of these buttons, but that looks like shit
			if (keyboardState[SDL_SCANCODE_W]) {
				move_dir[2] = 1.0f;
			}
			if (keyboardState[SDL_SCANCODE_A]) {
				move_dir[0] = 1.0f;
			}
			if (keyboardState[SDL_SCANCODE_S]) {
				move_dir[2] = -1.0f;
			}
			if (keyboardState[SDL_SCANCODE_D]) {
				move_dir[0] = -1.0f;
			}
			if (keyboardState[SDL_SCANCODE_Q]) {
				move_dir[1] = 1.0f;
			}
			if (keyboardState[SDL_SCANCODE_E]) {
				move_dir[1] = -1.0f;
			}
		}
		glm_vec3_normalize(move_dir);
		glm_vec3_scale(move_dir, movespeed*delta_t, move_dir);
		glm_vec3_add(camera_pos, move_dir, camera_pos);

		// UPDATE 
		// ------

		// Update all data for the particles
		
		int particle_count = 0;
		for (int i = 0; i < max_particles; ++i) {
			struct Particle* p = &particle_container[i];

			if (physics) {
			vec3 dir_to_middle;
				glm_vec3_negate_to(p->pos, dir_to_middle);

				glm_vec3_normalize(dir_to_middle);
				
				glm_vec3_scale(dir_to_middle, particle_accel*delta_t, dir_to_middle);

				glm_vec3_add(p->speed, dir_to_middle, p->speed);
				glm_vec3_add(p->speed, p->pos, p->pos);
			}

			g_particle_position_size_data[4*particle_count+0] = p->pos[0];
			g_particle_position_size_data[4*particle_count+1] = p->pos[1];
			g_particle_position_size_data[4*particle_count+2] = p->pos[2];

			g_particle_position_size_data[4*particle_count+3] = p->size;

			g_particle_color_data[4*particle_count+0] = p->r;
			g_particle_color_data[4*particle_count+1] = p->g;
			g_particle_color_data[4*particle_count+2] = p->b;
			g_particle_color_data[4*particle_count+3] = p->a;
			
			++particle_count;
		}

		// This is more effective than rewriting the buffer without reallocating it.
		// Link to explanation: https://www.khronos.org/opengl/wiki/Buffer_Object_Streaming
		//
		glBindBuffer(GL_ARRAY_BUFFER, particle_position_buffer);
		glBufferData(
			GL_ARRAY_BUFFER, 
			max_particles * 4 * sizeof(float), 
			NULL, 
			GL_STREAM_DRAW
		); // Buffer orphaning
		glBufferSubData(
			GL_ARRAY_BUFFER, 
			0, 
			particle_count * sizeof(float) * 4, 
			g_particle_position_size_data
		);

		glBindBuffer(GL_ARRAY_BUFFER, particle_color_buffer);
		glBufferData(
			GL_ARRAY_BUFFER, 
			max_particles * 4 * sizeof(unsigned char), 
			NULL, 
			GL_STREAM_DRAW
		); 
		glBufferSubData(
			GL_ARRAY_BUFFER, 
			0, 
			particle_count * sizeof(unsigned char) * 4, 
			g_particle_color_data
		);

		// Push the Vertex Attrib Arrays
		// -----------------------------
		// 1st attribute buffer: vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, particle_vertex_buffer);
		glVertexAttribPointer(
			0,
			3,
			GL_FLOAT,
			GL_FALSE,
			0,
			(void*)0
		);

		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, particle_position_buffer);
		glVertexAttribPointer(
			1,
			4,
			GL_FLOAT,
			GL_FALSE,
			0,
			(void*)0
		);

		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, particle_color_buffer);
		glVertexAttribPointer(
			2,
			4,
			GL_UNSIGNED_BYTE,
			GL_TRUE,
			0,
			(void*)0
		);

		glm_look(camera_pos, camera_dir, GLM_YUP, view);
		uniform3f(program, "cameraRight_worldspace", (vec3){1.0f, 0.0f, 0.0f}); // might change
		uniform3f(program, "cameraUp_worldspace", GLM_YUP);
		glm_mat4_mul(proj, view, vp);
		uniformMatrix4fv(program, "VP", vp);

		// RENDERING
		// ---------
		glUseProgram(program);

		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glBindTexture(GL_TEXTURE_2D, tex);
		
		// First argument specifies index of vertex attrib and second argument 
		// specifies how the buffer advances for every instance
		// Docs: https://docs.gl/gl3/glVertexAttribDivisor
		glVertexAttribDivisor(0, 0);
		glVertexAttribDivisor(1, 1);
		glVertexAttribDivisor(2, 1);

		glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, particle_count);

		SDL_GL_SwapWindow(window);

		{
			last_t = now_t;
			now_t = SDL_GetPerformanceCounter();
			delta_t = (double)((now_t - last_t)*1000) / SDL_GetPerformanceFrequency(); // in ms
		}
	}

	// DESTRUCTION
	// ===========
	free(particle_container);
	free(g_particle_color_data);
	free(g_particle_position_size_data);

	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}

float rand_float() {
	return (float)rand()/(float)RAND_MAX;
}

// I could expand this
void debugCallback(GLenum source, GLenum type, GLuint id,
		GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
	fprintf(stderr, "OpenGL Debug Message: %s\n", message);
}
