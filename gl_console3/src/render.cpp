#include "console.h"

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

GLuint prepShd() {
	GLuint p = glCreateProgram();

	GLuint vert = glCreateShader(GL_VERTEX_SHADER);
	const char *vertSrc = "#version 430 core\nlayout(location=0) in int currv;layout(location = 1)uniform vec2 u_pos[4];void main(){gl_Position = vec4(u_pos[currv], 0, 1);}";
	glShaderSource(vert, 1, &vertSrc, nullptr);
	glCompileShader(vert);
	checkSHDErr(vert, true);
	glerr();
	LOG("vert done");

	GLuint frag = glCreateShader(GL_FRAGMENT_SHADER);
	const char *fragSrc = "#version 430 core\nlayout(location = 0)uniform vec4 u_color;out vec4 color;void main(){color=u_color;}";
	glShaderSource(frag, 1, &fragSrc, nullptr);
	glCompileShader(frag);
	checkSHDErr(frag, true);
	glerr();
	LOG("frag done");

	glAttachShader(p, vert);
	glAttachShader(p, frag);
	glLinkProgram(p);
	glDeleteShader(vert);
	glDeleteShader(frag);
	checkSHDErr(p, false);
	glerr();
	LOG("link done");

	return p;
}

void Console::prepRenderer() {

	SHD = prepShd();

	glCreateVertexArrays(1, &VAO);
	glCreateBuffers(1, &VBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	int buffData[] = { 0, 1, 2, 3 };
	glBufferData(GL_ARRAY_BUFFER, sizeof(buffData), buffData, GL_STATIC_DRAW);

	glVertexAttribIPointer(0, 1, GL_INT, 0, nullptr);
	glEnableVertexAttribArray(0);

	glerr();
	LOG("prep render done");

}

void drawQuad(glm::vec2 pos1, glm::vec2 pos2, glm::vec4 color) {

	glBindVertexArray(1);
	glUseProgram(1);

	glUniform4f(0, color.x, color.y, color.z, color.w);

	glUniform2f(1, pos1.x, pos1.y);
	glUniform2f(2, pos1.x, pos2.y);
	glUniform2f(3, pos2.x, pos2.y);
	glUniform2f(4, pos2.x, pos1.y);

	glDrawArrays(GL_QUADS, 0, 4);

	glerr();

}

void Console::drawIntf() {

	drawQuad({ -wWh, wHh }, { 0, -wHh }, { .36, .36, .6, 1 });

	glfwSwapBuffers(wnd);

}