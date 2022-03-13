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

void Console::prepRenderer() {

	prepShd("#version 430 core\nlayout(location=0) in int currv;layout(location = 1)uniform vec2 u_pos[4];void main(){gl_Position = vec4(u_pos[currv], 0, 1);}",
		"#version 430 core\nlayout(location = 0)uniform vec4 u_color;out vec4 color;void main(){color=u_color;}");

	GLuint tmp = 0;

	glCreateVertexArrays(1, &tmp);
	glCreateBuffers(1, &tmp);

	glBindVertexArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, 1);
	int buffData[] = { 0, 1, 2, 3 };
	glBufferData(GL_ARRAY_BUFFER, sizeof(buffData), buffData, GL_STATIC_DRAW);

	glVertexAttribIPointer(0, 1, GL_INT, 0, nullptr);
	glEnableVertexAttribArray(0);

	GLERR;
	LOG("prep render done");

}

void drawQuad(glm::vec2 pos1, glm::vec2 pos2, Color color) {

	glBindVertexArray(1);
	glUseProgram(1);

	glUniform4f(0, color.r, color.g, color.b, color.a);

	glUniform2f(1, pos1.x, pos1.y);
	glUniform2f(2, pos1.x, pos2.y);
	glUniform2f(3, pos2.x, pos2.y);
	glUniform2f(4, pos2.x, pos1.y);

	glDrawArrays(GL_QUADS, 0, 4);

	GLERR;

}

void Console::drawIntf() {

	// left bar
	drawQuad({ -1, 1 }, { -1 + layout.bar_left, -1 }, layout.bar_color);

	// right bar
	drawQuad({ 1, 1 }, { 1 - layout.bar_right, -1 }, layout.bar_color);

	// bottom bar
	drawQuad({ -1, -1 + layout.bar_bottom }, { 1, -1 }, layout.bar_color);
	
	// toolbar
	drawQuad({ -1, 1 }, { 1, 1 - layout.toolbar }, layout.toolbar_color);

	// input bg
	drawQuad({ -1 + layout.bar_left, -1 + layout.bar_bottom + layout.input_bg }, { 1 - layout.bar_right, -1 + layout.bar_bottom }, layout.input_bg_color);


}

