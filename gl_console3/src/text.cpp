#include <ft2build.h>
#include FT_FREETYPE_H

#include "console.h"
#include <unordered_map>
#include <list>

GLuint prepShd(const char *vertSrc, const char *fragSrc);

void Console::terminateText() {
	FT_Done_Face(face);
	FT_Done_FreeType(library);
}

struct CharInfo {
	GLuint tex = 0;
	int w = 0, h = 0, l = 0, t = 0, a = 0;
	std::list<glm::vec4> pos;
};

typedef std::unordered_map<char, CharInfo> charmap;

void Console::prepText() {
	
	FT_Init_FreeType(&library);

	if(FT_New_Face(library, "C:/Windows/Fonts/arial.ttf", 0, &face)) { LOG("an error occured while loading font face"); return; }

	FT_Set_Pixel_Sizes(face, 0, 16);

	prepShd(
		R"(#version 430 core
		layout(location=0) in vec2 pos;
		layout(location=1) in int tex;
		layout(location=2) in int color;
		flat out ivec2 tc;
		out vec2 texCoord;
		void main() {
			tc = ivec2(tex, color);
			gl_Position = vec4(pos, 0, 1);
			switch(gl_VertexID % 4) {
				case 0: texCoord = vec2(1, 0); break;
				case 1: texCoord = vec2(1, 1); break;
				case 2: texCoord = vec2(0, 1); break;
				case 3: texCoord = vec2(0, 0); break;
			}
		})"
	,
		R"(#version 430 core
		flat in ivec2 tc;
		in vec2 texCoord;
		out vec4 color;
		layout(location=5)uniform sampler2D sampler[32];
		void main() {
			color = vec4(1, 1, 1, texture(sampler[tc.x], texCoord).r);

			//color = vec4(texCoord, 0, 1);
		}
		)");
	for(int i = 0; i < 32; i++) glUniform1iv(5 + i, 1, &i);

}

charmap buildCharmap(const char *text) {
	charmap ret;
	for(size_t i = 0; text[i]; i++) ret[text[i]];
	return std::move(ret);
}

void setTextures(FT_Face &face, charmap *m) {
	for(auto &it : *m) {
		CharInfo &ci = it.second;
		if(FT_Load_Char(face, it.first, FT_LOAD_RENDER)) {
			LOG("failed to load glyph '%c'", it.first);
			continue;
		}
		FT_GlyphSlot sl = face->glyph;
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		// generate texture
		glGenTextures(1, &ci.tex);
		glBindTexture(GL_TEXTURE_2D, ci.tex);
		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_RED,
			sl->bitmap.width,
			sl->bitmap.rows,
			0,
			GL_RED,
			GL_UNSIGNED_BYTE,
			sl->bitmap.buffer
		);
		// set texture options
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		ci.w = sl->bitmap.width;
		ci.h = sl->bitmap.rows;
		ci.l = sl->bitmap_left;
		ci.t = sl->bitmap_top;
		ci.a = sl->advance.x;
	}
}

void setPositions(charmap *m, const char *text) {

	float scale = .1f;
	float lineHeight = .1f;

	float advance = 0;
	size_t line = 0;

	float x = 0, y = 0, w = 0, h = 0;

	char c = 0;
	for(size_t i = 0; c = text[i]; i++) {
		auto found = m->find(c);
		if(found == m->end()) continue;
		CharInfo *ci = &found->second;

		x = advance * scale;
		y = line * lineHeight;
		w = ci->w * scale;
		h = ci->h * scale;

		ci->pos.push_back({x, y, x + w, y - h});

		advance += (ci->a >> 6);
		if(advance > 100) { line++; advance = 0; }

	}
}

#pragma pack(push, 1)
struct Vertex {
	float x = 0, y = 0;
	int tex = 0;
	int color = 0;
	Vertex() {}
	Vertex(float x, float y, int tex, int color): x(x), y(y), tex(tex), color(color) {}
};
#pragma pack(pop)

void setBuffers(charmap *m, const char *text) {
	GLuint VAO = 0, VBO = 0;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	size_t totalCharCount = 0;
	for(auto &it : *m) totalCharCount += it.second.pos.size();

	if(!totalCharCount) return;

	Vertex *verts = new Vertex[totalCharCount * 4];
	size_t vert_counter = 0;

	char c = 0;
	for(size_t i = 0; c = text[i]; i++) {
		auto found = m->find(c);
		if(found == m->end()) continue;
		CharInfo *ci = &found->second;

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, ci->tex);

		verts[vert_counter * 4 + 0] = Vertex(0, 0, 0, ~0);
		verts[vert_counter * 4 + 1] = Vertex(0, -.5f, 0, ~0);
		verts[vert_counter * 4 + 2] = Vertex(.5f, -.5f, 0, ~0);
		verts[vert_counter * 4 + 3] = Vertex(.5f, 0, 0, ~0);

		vert_counter++;

		break;

	}

	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * totalCharCount * 4, verts, GL_STATIC_DRAW);
	delete[]verts;

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void *>(0));
	glVertexAttribIPointer(1, 1, GL_INT, sizeof(Vertex), reinterpret_cast<void *>(sizeof(float) * 2));
	glVertexAttribIPointer(2, 1, GL_INT, sizeof(Vertex), reinterpret_cast<void *>(sizeof(float) * 2 + sizeof(int)));

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

}

void Console::drawText() {

	int x = static_cast<int>(layout.bar_left * wWh), y = static_cast<int>(layout.bar_bottom * wHh + layout.input_bg * wHh);

	glViewport(x, y, wW - x - layout.bar_right * wWh, wH - y - layout.toolbar * wHh);


	glUseProgram(4);


	const char *tx = "Hbcd test";

	charmap chMap = buildCharmap(tx);

	setTextures(face, &chMap);

	setPositions(&chMap, tx);

	setBuffers(&chMap, tx);

	glDrawArrays(GL_QUADS, 0, 16);

	GLERR;

	glViewport(0, 0, wW, wH);

}

