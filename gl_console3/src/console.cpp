#include "console.h"

void glerr() {
	int e = 0;
	while(e = glGetError()) LOG("GL ERROR: %i", e);
}

Console::Console(uint32_t w, uint32_t h): wW(w), wH(h), wWh(w/2), wHh(h/2) {}

void Console::create(uint32_t w, uint32_t h) { this->wW = w, this->wH = h; this->wWh = w/2, this->wHh = h/2; create(); }

Console *getWUP(GLFWwindow *window) { return reinterpret_cast<Console *>(glfwGetWindowUserPointer(window)); }

void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) { getWUP(window)->clb_key(key, scancode, action, mods); }
void curposCallback(GLFWwindow *window, double xpos, double ypos) { getWUP(window)->clb_curpos(xpos, ypos); }

void Console::create() {
	
	if(wnd) return;

	wnd = glfwCreateWindow(wW, wH, "console", nullptr, nullptr);
	glfwMakeContextCurrent(wnd);

	glfwSetWindowUserPointer(wnd, this);

	glewInit();

	glViewport(0, 0, wW, wH);

	glfwSetKeyCallback(wnd, keyCallback);
	glfwSetCursorPosCallback(wnd, curposCallback);

	glClearColor(.6, .6, .6, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	prepRenderer();

	repaint();

}

void Console::uptade() {

}

void Console::repaint() {

	glfwMakeContextCurrent(wnd);
	drawIntf();

}

void Console::clb_key(int key, int scancode, int action, int mods) {}

void Console::clb_curpos(double xpos, double ypos) {}

