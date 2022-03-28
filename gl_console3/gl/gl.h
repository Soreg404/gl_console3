#pragma once

#include <glew/glew.h>

#include <glfw/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


#include <iostream>
#ifndef LOG
#define LOG(x, ...) printf("[" __FILE__ "] %i: " x "\n", __LINE__, __VA_ARGS__);
#endif

void glerr(const char *file, int line);
#define GLERR glerr(__FILE__, __LINE__)


void checkSHDErr(GLuint shd, bool cmp);

GLuint prepShd(const char *vertSrc, const char *fragSrc);
