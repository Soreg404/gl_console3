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
void charCallback(GLFWwindow *window, unsigned int codepoint) { getWUP(window)->clb_char(codepoint); }

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
	glfwSetCharCallback(wnd, charCallback);

	glClearColor(.6f, .6f, .6f, 1.f);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	prepInterface();

	prepText();

	repaint();

}

void Console::close() {
	glfwDestroyWindow(wnd);
	wnd = nullptr;
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


void Console::prepText() {

	glProvokingVertex(GL_LAST_VERTEX_CONVENTION);

	lib = std::make_shared<Library>();
	font = std::make_shared<Font>(lib);

	//font->load("C:/Windows/Fonts/BIZ-UDGothicR.ttc");
	font->load("C:/Windows/Fonts/consola.ttf");

	font->setSize(0, 16);


	input.text(font, inputData.c_str());
	input.repos(400, 16);

}


void Console::parseLogEntry(const wchar_t *tmpLogText) {
	TextField tmp;

	tmp.text(font, tmpLogText);
	tmp.repos(wW - layout.bar_left - layout.bar_right, 16);

	logBuffer.push_back(std::move(tmp));
}

float scl = 1.f;
void Console::resize(uint32_t width, uint32_t height) {
	glfwMakeContextCurrent(wnd);
	wW = width, wWh = width / 2, wH = height, wHh = height / 2;
	proj[0] = glm::ortho(0.f, static_cast<float>(wW), 0.f, static_cast<float>(wH), 1.f, 10.f);
	glViewport(0, 0, wW, wH);

	for(auto &e : logBuffer) {
		e.repos(wW - layout.bar_left - layout.bar_right, 16, scl);
	}
}

int tmpLog = 0;
std::array<const wchar_t *, 6>tmpTexts = {
	L"ga luhdglohal4woe olitayuo4iyu to9i8yu e5oq89490pq808ypo5ep 5p94wy9p5u49wuy4o5jypw45k'u;pok'piu-40oyk4k5ioy [epr     śśłłęśńds gjnś ś",
	L"adlr",
	L"sample text",
	L"Lorem ipsum sit amet dolor",
	L"zażółć gęślą jaźń",
	L"猫がいる",
};

void Console::clb_key(int key, int scancode, int action, int mods) {

	if(key == GLFW_KEY_ENTER && action) {
		parseLogEntry(tmpTexts[glfwGetTimerValue() % tmpTexts.size()]);
	}/* else if(key == GLFW_KEY_R && action) {
		scl += .03f;
	} else if(key == GLFW_KEY_T && action) {
		scl -= .03f;
	} else if(key == GLFW_KEY_C && action) {
		logBuffer.clear();
	}*/

	for(auto &e : logBuffer) {
		e.repos(wW - layout.bar_left - layout.bar_right, 16, scl);
	}

	repaint();
}

void Console::clb_curpos(double xpos, double ypos) {}

void Console::clb_mouse(int button, int action, int mods) {}

void Console::clb_size(int width, int height) { resize(width, height); repaint(); }

void Console::clb_char(char32_t codepoint) {
	if(codepoint > 0xD800) return;
	inputData += static_cast<wchar_t>(codepoint);
	input.text(font, inputData.c_str());
	input.repos(400, 16);
}
