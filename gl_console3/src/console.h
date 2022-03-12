#pragma once

#define GLEW_STATIC
#include "../gl/ext_gl.h"

typedef __int32 int32_t;
typedef unsigned __int32 uint32_t;

#include <iostream>

#ifndef LOG
#define LOG(x, ...) printf("[" __FILE__ "] %i: " x "\n", __LINE__, __VA_ARGS__);
#endif

void glerr();

class Console {

public:

	Console() = default;
	Console(uint32_t width, uint32_t height);

	void create(uint32_t width, uint32_t height);
	void create();

	void uptade();

	void repaint();

	inline operator bool() { return wnd; }


	struct LAYOUT {

	} layout;

private:

	GLFWwindow *wnd = nullptr;

	int wW = 400, wH = 600, wWh = 200, wHh = 300;

#pragma region callbacks

	friend void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
	void clb_key(int key, int scancode, int action, int mods);

	friend void curposCallback(GLFWwindow *window, double xpos, double ypos);
	void clb_curpos(double xpos, double ypos);

#pragma endregion

	GLuint VAO = 0, VBO = 0, SHD = 0;

	void prepRenderer();

	void drawIntf();

};