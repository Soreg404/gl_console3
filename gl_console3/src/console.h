#pragma once

#define GLEW_STATIC
#include "../gl/ext_gl.h"

typedef __int32 int32_t;
typedef unsigned __int32 uint32_t;

#include <iostream>

void glerr(const char *file, int line);

#ifndef LOG
#define LOG(x, ...) printf("[" __FILE__ "] %i: " x "\n", __LINE__, __VA_ARGS__);
#endif

#define GLERR glerr(__FILE__, __LINE__)

struct Color {
	float r = 0, g = 0, b = 0, a = 1;
	Color(float r, float g, float b, float a);
	Color(int r, int g, int b, int a);
	Color(float all);
	Color(int all);
};

class Console {

public:

	Console() = default;
	Console(uint32_t width, uint32_t height);
	~Console();

	void create(uint32_t width, uint32_t height);
	void create();

	void close();

	void uptade();

	void repaint();

	operator bool();


	struct LAYOUT {
		float bar_left = .2f;
		float bar_right = .2f;
		float bar_bottom = .2f;
		Color bar_color{ 61, 75, 88, 255 };
		float toolbar = .2f;
		Color toolbar_color{ 38, 87, 113, 255 };
		float input_bg = .2f;
		Color input_bg_color{ 71, 85, 94, 255 };
	} layout;

private:

	GLFWwindow *wnd = nullptr;

	int wW = 400, wH = 600, wWh = 200, wHh = 300;

#pragma region callbacks

	friend void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
	void clb_key(int key, int scancode, int action, int mods);

	friend void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods);
	void clb_mouse(int button, int action, int mods);

	friend void curposCallback(GLFWwindow *window, double xpos, double ypos);
	void clb_curpos(double xpos, double ypos);
	
	friend void sizeCallback(GLFWwindow *window, int width, int height);
	void clb_size(int width, int height);


#pragma endregion

	void prepRenderer();

	void drawIntf();

	void drawText();

	struct FT_LibraryRec_ *library = nullptr;
	struct FT_FaceRec_ *face = nullptr;
	void prepText();
	void terminateText();

};