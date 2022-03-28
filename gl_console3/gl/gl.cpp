#include "gl.h"

void glerr(const char *file, int line) {
	int e = 0;
	while(e = glGetError()) printf("[%s] %i: GL ERROR: %i\n", file, line, e);
}

void checkSHDErr(GLuint shd, bool cmp) {
	int success = 0;
	if(cmp) glGetShaderiv(shd, GL_COMPILE_STATUS, &success);
	else glGetProgramiv(shd, GL_LINK_STATUS, &success);

	if(!success) {
		char infoLog[512] = { 0 };
		if(cmp) glGetShaderInfoLog(shd, 512, NULL, infoLog);
		else glGetProgramInfoLog(shd, 512, NULL, infoLog);
		LOG("error compiling shader:\n%s\n", infoLog);
	}
}

GLuint prepShd(const char *vertSrc, const char *fragSrc) {
	GLuint p = glCreateProgram();

	GLuint vert = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vert, 1, &vertSrc, nullptr);
	glCompileShader(vert);
	checkSHDErr(vert, true);
	GLERR;
	LOG("vert done");

	GLuint frag = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(frag, 1, &fragSrc, nullptr);
	glCompileShader(frag);
	checkSHDErr(frag, true);
	GLERR;
	LOG("frag done");

	glAttachShader(p, vert);
	glAttachShader(p, frag);
	glLinkProgram(p);
	glDeleteShader(vert);
	glDeleteShader(frag);
	checkSHDErr(p, false);
	GLERR;
	LOG("link done");
	return p;
}