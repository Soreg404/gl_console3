#include <ft2build.h>
#include FT_FREETYPE_H

#include "console.h"
#include "shdSrc/text.h"

char TMPSTR[1000] = { 0 };

int TEXTURE_LIMIT = 1;

void Console::prepText() {
	
	FT_Init_FreeType(&library);

	font.set(library, "C:/Windows/Fonts/BIZ-UDGothicR.ttc");
	//font.set(library, "C:/Windows/Fonts/consolas.ttf");
	FT_Set_Pixel_Sizes(font.face, 0, 60);

	glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &TEXTURE_LIMIT);
	LOG("texture units: %i", TEXTURE_LIMIT);

	sprintf_s(TMPSTR, 1000, shdSrc::fragment, TEXTURE_LIMIT);
	prepShd(shdSrc::vertex, TMPSTR);

	glUseProgram(4);
	for(int i = 0; i < TEXTURE_LIMIT; i++) glUniform1iv(20 + i, 1, &i);

}

void Console::parseLogEntry(const wchar_t *tmpLogText) {
	TextField tmp;
	tmp.width = wW - layout.bar_left - layout.bar_right;
	tmp.scale = .8f;
	tmp.lineHeight = 60 * tmp.scale;

	tmp.set(font, tmpLogText);

	logs.push_back(std::move(tmp));
}

size_t dc = 0;
void drawTextContainer(TextField *tc) {

	auto mapIt = tc->map.begin();
	size_t mapCounter = 0;

	for(size_t i = 0; i < tc->buffers.size(); i++) {
		
		glBindVertexArray(tc->buffers[i]);

		// bind textures
		for(size_t t = 0; t < TEXTURE_LIMIT && mapCounter < tc->map.size(); t++, mapIt++, mapCounter++) {
			glActiveTexture(GL_TEXTURE0 + t);
			glBindTexture(GL_TEXTURE_2D, mapIt->second.tex);
		}

		//LOG("DRAW CALL %zu", dc);
		dc++;
		glDrawArrays(GL_QUADS, 0, 600);
	}

}

void Console::drawText() {

	int x = layout.bar_left, y = layout.bar_bottom + layout.input_bg;
	int vW = wW - x - layout.bar_right, vH = wH - y - layout.toolbar;
	glViewport(x, y, vW, vH);

	GLERR;

	glUseProgram(4);

	GLERR;

	glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(glm::ortho(0.f, static_cast<float>(vW), 0.f, static_cast<float>(vH), 1.f, 10.f)));
	

	GLERR;

	int yPos = 0;
	for(auto it = logs.rbegin(); it != logs.rend(); it++) {
		if(yPos > vH) break;
		yPos += it->lineHeight * it->lines + 16;
		glUniform2f(1, 0, yPos);
		drawTextContainer(&(*it));
	}

	GLERR;

	glViewport(0, 0, wW, wH);

}


charmap buildCharmap(const wchar_t *text) {
	charmap ret;
	for(size_t i = 0; text[i]; i++) ret[text[i]];
	return std::move(ret);
}

CharInfo loadCharTex(FT_Face &face, wchar_t c) {
	if(FT_Load_Char(face, c, FT_LOAD_RENDER)) {
		LOG("failed to load glyph '%c'", c);
		return {};
	}
	CharInfo ret;
	FT_GlyphSlot sl = face->glyph;
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	// generate texture
	glGenTextures(1, &ret.tex);
	glBindTexture(GL_TEXTURE_2D, ret.tex);
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

	ret.w = sl->bitmap.width;
	ret.h = sl->bitmap.rows;
	ret.l = sl->bitmap_left;
	ret.t = sl->bitmap_top;
	ret.a = sl->advance.x;

	return ret;
}
void assignTextures(Font &face, charmap *m) {
	for(auto &it : *m) {
		CharInfo *currTexName = &(face.index)[it.first];
		if(!currTexName->tex) *currTexName = loadCharTex(face.face, it.first);
		it.second = *currTexName;
	}
}

void setPositions(TextField *cont, const wchar_t *text, float scale, int width, int lineHeight) {

	charmap *m = &cont->map;

	float advance = 0;
	int line = 1;

	float x = 0, y = 0, w = 0, h = 0;

	wchar_t c = 0;
	for(size_t i = 0; c = text[i]; i++) {
		auto found = m->find(c);
		if(found == m->end()) continue;
		CharInfo *ci = &found->second;

		if((advance + ci->w) * scale > width) { line++; advance = 0; }

		x = (advance + ci->l) * scale;
		y = -(line * lineHeight) + ci->t * scale;
		w = ci->w * scale;
		h = ci->h * scale;

		ci->pos.push_back({ x, y, w, h });

		advance += (ci->a >> 6);

	}
	cont->width = 200;
	cont->lines = line;
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
void setBuffers(TextField *cont) {
	
	cont->buffers = std::vector<GLuint>(cont->map.size() / TEXTURE_LIMIT + 1);

	auto mapIt = cont->map.begin(), mapIt2 = mapIt;
	size_t mapCounter = 0, mapCounter2 = 0;


	for(size_t i = 0; i < cont->buffers.size(); i++) {
		glGenVertexArrays(1, &cont->buffers[i]);
		glBindVertexArray(cont->buffers[i]);
		GLuint VBO = 0;
		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);

		size_t totalCharCount = 0;
		for(size_t t = 0; t < TEXTURE_LIMIT && mapCounter2 < cont->map.size(); t++, mapIt2++, mapCounter2++) { // <- ¯\_(ツ)_/¯
			totalCharCount += mapIt2->second.pos.size();
		}
		if(!totalCharCount) continue;

		Vertex *verts = new Vertex[4 * totalCharCount];
		size_t vert_counter = 0;

		for(size_t t = 0; t < TEXTURE_LIMIT && mapCounter < cont->map.size(); t++, mapIt++, mapCounter++) { // <- ¯\_(ツ)_/¯ x2

			for(auto &cp : mapIt->second.pos) {
				verts[vert_counter * 4 + 0] = Vertex(cp.x, cp.y, t, ~0);
				verts[vert_counter * 4 + 1] = Vertex(cp.x, cp.y - cp.w, t, ~0);
				verts[vert_counter * 4 + 2] = Vertex(cp.x + cp.z, cp.y - cp.w, t, ~0);
				verts[vert_counter * 4 + 3] = Vertex(cp.x + cp.z, cp.y, t, ~0);
				vert_counter++;
			}
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


}


void TextField::set(Font &face, const wchar_t *ndata) {
	if(data) delete[]data;
	size_t len = wcslen(ndata) + 1;
	data = new wchar_t[len];
	wcscpy_s(data, len, ndata);

	map = buildCharmap(data);
	assignTextures(face, &map);

	setPositions(this, data, scale, width, lineHeight);

	setBuffers(this);

}


void Console::terminateText() {
	FT_Done_Face(font.face);
	FT_Done_FreeType(library);
}

void Font::set(struct FT_LibraryRec_ *library, const char *path) {
	if(!FT_New_Face(library, path, 0, &face)) {
		index.clear();
	} else LOG("an error occured while loading font face");
}
