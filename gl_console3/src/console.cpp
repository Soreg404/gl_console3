#include "console.h"


Color::Color(float r, float g, float b, float a) : r(r), g(g), b(b), a(a) {}
Color::Color(int r, int g, int b, int a) : r(r / 255.f), g(g / 255.f), b(b / 255.f), a(a / 255.f) {}
Color::Color(float all) : r(all), g(all), b(all), a(1) {}
Color::Color(int all) : r(all / 255.f), g(all / 255.f), b(all / 255.f), a(1) {}

Console::Console() {
	for(auto &i : proj) i = glm::mat4(1.f);
}

Console::~Console() {
}

Console *getWUP(GLFWwindow *window) { return reinterpret_cast<Console *>(glfwGetWindowUserPointer(window)); }

void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) { getWUP(window)->clb_key(key, scancode, action, mods); }
void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods) { getWUP(window)->clb_mouse(button, action, mods); }
void curposCallback(GLFWwindow *window, double xpos, double ypos) { getWUP(window)->clb_curpos(xpos, ypos); }
//void closeCallback(GLFWwindow *window) { getWUP(window)->close(); }
void sizeCallback(GLFWwindow *window, int width, int height) { getWUP(window)->clb_size(width, height); }

void Console::create(uint32_t w, uint32_t h) {

	if(wnd) return;

	wnd = glfwCreateWindow(w, h, "console", nullptr, nullptr);
	glfwMakeContextCurrent(wnd);

	glfwSetWindowUserPointer(wnd, this);

	glewInit();

	glfwSetWindowSizeLimits(wnd, 150, 200, -1, -1);
	resize(w, h);

	glfwSetKeyCallback(wnd, keyCallback);
	glfwSetMouseButtonCallback(wnd, mouseButtonCallback);
	glfwSetCursorPosCallback(wnd, curposCallback);
	//glfwSetWindowCloseCallback(wnd, closeCallback);
	glfwSetWindowSizeCallback(wnd, sizeCallback);

	glClearColor(.6f, .6f, .6f, 1.f);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	prepInterface();

	prepText();

	repaint();

}

void Console::close() {
	glfwDestroyWindow(wnd);
}

void Console::uptade() {

}

void Console::repaint() {

	glfwMakeContextCurrent(wnd);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	drawIntf();

	drawText();

	glfwSwapBuffers(wnd);


}

Console::operator bool() {
	if(glfwWindowShouldClose(wnd)) { glfwDestroyWindow(wnd); wnd = nullptr; }
	return wnd;
}


void Console::resize(uint32_t width, uint32_t height) {
	glfwMakeContextCurrent(wnd);
	wW = width, wWh = width / 2, wH = height, wHh = height / 2;
	proj[0] = glm::ortho(0.f, static_cast<float>(wW), 0.f, static_cast<float>(wH), 1.f, 10.f);
	glViewport(0, 0, wW, wH);

	for(auto &e : buffer) {
		e.repos(wW - layout.bar_left - layout.bar_right, 16);
	}
}

int tmpLog = 0;
std::array<const wchar_t *, 5>tmpTexts = {
	L"hello world",
	L"sample text",
	L"Lorem ipsum sit amet dolor",
	L"zażółć gęślą jaźń",
	L"猫がいる",
};

void Console::clb_key(int key, int scancode, int action, int mods) {

	if(key == GLFW_KEY_ENTER && action) {
		parseLogEntry(tmpTexts[glfwGetTimerValue() % tmpTexts.size()]);
		repaint();
	}

}

void Console::clb_curpos(double xpos, double ypos) {}

void Console::clb_mouse(int button, int action, int mods) {}

void Console::clb_size(int width, int height) { resize(width, height); repaint(); }
