#pragma once

#define GLEW_STATIC
#include "../gl/gl.h"

#include "text.h"

class Console {

public:

	Console();
	~Console();

	void create(uint32_t width, uint32_t height);

	void close();

	void uptade();

	void repaint();

	operator bool();

	void resize(uint32_t width, uint32_t height);



	struct LAYOUT {
		int bar_left = 20;
		int bar_right = 20;
		int bar_bottom = 50;
		Color bar_color{ 61, 75, 88, 255 };
		int toolbar = 30;
		Color toolbar_color{ 38, 87, 113, 255 };
		int input_bg = 30;
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

	void prepInterface();
	void prepText();

	void drawIntf();
	void drawText();

	std::shared_ptr<Library> lib;

	std::shared_ptr<Font> font;

	std::list<TextField> buffer;

	std::array<glm::mat4, 2> proj;

	void parseLogEntry(const wchar_t *tmpLogText);

};