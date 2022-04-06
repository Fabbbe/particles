# OpenGL Particles

This project is rendering many smaller optimized particles through instancing in OpenGL. Instancing is probably the most effective way to generate particles. The other option not used here is using one VBO to keep track of all different particles, but this is space inefficient since it creates a large VBO.

## Dependencies

+ [glew](http://glew.sourceforge.net/)
+ [SDL2](https://www.libsdl.org/download-2.0.php)
+ [cglm](https://github.com/recp/cglm)

## Commands 
To make the code on linux:
```
$ make run
```
