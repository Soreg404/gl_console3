#include "console.h"

void Console::prepInterface() {

	prepShd("#version 430 core\nlayout(location=0) in int currv;layout(location = 0)uniform mat4 proj = mat4(1); layout(location = 2)uniform vec2 u_pos[4];void main(){gl_Position = proj * vec4(u_pos[currv], -5, 1);}",
		"#version 430 core\nlayout(location = 1)uniform vec4 u_color;out vec4 color;void main(){color=u_color;}");

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
	LOG("prep interface done");

}

void drawQuad(glm::vec2 pos1, glm::vec2 pos2, Color color);

void Console::drawIntf() {

	glUseProgram(1);

	glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(proj[0]));

	// left bar
	drawQuad({ 0, wH }, { layout.bar_left, 0 }, layout.bar_color);

	// right bar
	drawQuad({ wW- layout.bar_right, wH }, { wW , 0 }, layout.bar_color);

	// bottom bar
	drawQuad({ 0, layout.bar_bottom }, { wW, 0 }, layout.bar_color);
	
	// toolbar
	drawQuad({ 0, wH }, { wW, wH - layout.toolbar }, layout.toolbar_color);

	// input bg
	drawQuad({ layout.bar_left, layout.bar_bottom + layout.input_bg }, { wW - layout.bar_right, layout.bar_bottom }, layout.input_bg_color);


}


void drawQuad(glm::vec2 pos1, glm::vec2 pos2, Color color) {

	glBindVertexArray(1);
	glUseProgram(1);

	glUniform4f(1, color.r, color.g, color.b, color.a);

	glUniform2f(2, pos1.x, pos1.y);
	glUniform2f(3, pos1.x, pos2.y);
	glUniform2f(4, pos2.x, pos2.y);
	glUniform2f(5, pos2.x, pos1.y);

	glDrawArrays(GL_QUADS, 0, 4);

	GLERR;

}
