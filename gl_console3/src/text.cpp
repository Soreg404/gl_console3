#include <ft2build.h>
#include FT_FREETYPE_H

#include "console.h"
#include "shdSrc/text.h"

char TMPSTR[1000] = { 0 };

const int MAX_CHARS_PER_BUFFER = 4000;

#pragma pack(push, 1)
struct Vertex {
	float x, y;
	int tex, color;
	Vertex(float x = 0, float y = 0, int tex = 0, int color = 0): x(x), y(y), tex(tex), color(color) {}
};

struct Indice {
	std::array<uint32_t, 6> v;
	Indice() { memset(v.data(), 0, 6 * sizeof(uint32_t)); }
	Indice(uint32_t b) {
		uint32_t t[] = { b + 0, b + 1, b + 3, b + 1, b + 2, b + 3 };
		memcpy_s(v.data(), 6 * sizeof(uint32_t), t, 6 * sizeof(uint32_t));
	}
};
#pragma pack(pop)

int TEXTURE_LIMIT = 1;

#define FS_

void Console::prepText() {
	
	glProvokingVertex(GL_LAST_VERTEX_CONVENTION);

	lib = std::make_shared<Library>();
	font = std::make_shared<Font>(lib);

	font->load("C:/Windows/Fonts/BIZ-UDGothicR.ttc");
	font->load("C:/Windows/Fonts/consola.ttf");

	font->setSize(0, 16);

	glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &TEXTURE_LIMIT);
	LOG("texture units: %i", TEXTURE_LIMIT);

	sprintf_s(TMPSTR, 1000, shdSrc::fragment, TEXTURE_LIMIT);
	prepShd(shdSrc::vertex, TMPSTR);

	glUseProgram(4);
	for(int i = 0; i < TEXTURE_LIMIT; i++) glUniform1iv(20 + i, 1, &i);

}

void Console::parseLogEntry(const wchar_t *tmpLogText) {
	TextField tmp;

	tmp.text(font, tmpLogText);
	tmp.repos(wW - layout.bar_left - layout.bar_right, 16);

	buffer.push_back(std::move(tmp));
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
	for(auto it = buffer.rbegin(); it != buffer.rend(); it++) {
		if(yPos > vH) break;
		yPos += it->getLineHeight() * it->getLines() + 16;
		glUniform2f(1, 0, yPos);
		it->draw();
	}

	GLERR;

	glViewport(0, 0, wW, wH);

}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


Font::Font(std::shared_ptr<Library> l): lib(l) {}

Font::~Font() { FT_Done_Face(face); }

void Font::load(const char *path) {
	FT_Done_Face(face);
	if(!FT_New_Face(lib->get(), path, 0, &face)) {
		index.clear();
	} else LOG("an error occured while loading font face");
}

void Font::setSize(uint32_t w, uint32_t h) { FT_Set_Pixel_Sizes(face, w, h); this->w = w; this->h = h; }

Library::Library() {
	FT_Init_FreeType(&library);
	glGenBuffers(1, &m_EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
	Indice *inds = new Indice[MAX_CHARS_PER_BUFFER];
	for(uint32_t i = 0, vbuffCounter = 0; i < MAX_CHARS_PER_BUFFER; i++, vbuffCounter += 4)
		inds[i] = { vbuffCounter };
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, MAX_CHARS_PER_BUFFER * 6 * sizeof(uint32_t), inds, GL_STATIC_DRAW);
	delete[]inds;
}

Library::~Library() { FT_Done_FreeType(library); }


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TextField::~TextField() {

}

size_t dc = 0;
void TextField::draw() {
	if(!font) return;

	auto mapIt = map.begin();
	size_t mapCounter = 0;

	for(size_t i = 0; i < buffers.size(); i++) {

		glBindVertexArray(buffers[i][0]);

		// bind textures
		for(size_t t = 0; t < TEXTURE_LIMIT && mapCounter < map.size(); t++, mapIt++, mapCounter++) {
			glActiveTexture(GL_TEXTURE0 + t);
			glBindTexture(GL_TEXTURE_2D, font->index[mapIt->first].tex);
		}

		//LOG("DRAW CALL %zu", dc);
		dc++;
		//glDrawArrays(GL_QUADS, 0, 600);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, font->lib->EBO());
		glDrawElements(GL_TRIANGLES, 200, GL_UNSIGNED_INT, (void *)0);
	}

}


void TextField::buildCharmap() { for(size_t i = 0; data[i]; i++) map[data[i]]; }

CharIndexed loadCharTex(FT_Face &face, wchar_t c) {
	if(FT_Load_Char(face, c, FT_LOAD_RENDER)) {
		LOG("failed to load glyph '%c'", c);
		return {};
	}
	CharIndexed ret;
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
void TextField::assignTextures() {
	for(auto &it : map) {
		// if texture for current char is just created - load it and store
		CharIndexed *currTexName = &(font->index)[it.first];
		if(!currTexName->tex) *currTexName = loadCharTex(font->face, it.first);
	}
}

void TextField::setPositions() {
	float advance = 0;
	wchar_t c = 0;
	for(size_t i = 0; c = data[i]; i++) {
		auto found = font->index.find(c);
		if(found == font->index.end()) continue;
		CharIndexed ci = found->second;

		CharMapped pos;

		if((advance + ci.w) * scale > width) { lines++; advance = 0; }

		pos.x = (advance + ci.l) * scale;
		pos.y = -((lines + 1) * lineHeight) + ci.t * scale;
		pos.w = ci.w * scale;
		pos.h = ci.h * scale;

		map[c].push_back(pos);

		advance += (ci.a >> 6);
	}
}

void TextField::setBuffers(bool repos /* tmp */) {
	
	// MAX_CHARS_PER_BUFFER ???
	if(!repos) buffers = std::vector<std::array<GLuint, 2>>(map.size() / TEXTURE_LIMIT + 1);

	auto mapIt = map.begin(), mapIt2 = mapIt;
	size_t mapCounter = 0, mapCounter2 = 0;


	for(size_t i = 0; i < buffers.size(); i++) {

		if(!repos) {
			glGenVertexArrays(1, &buffers[i][0]);
			glGenBuffers(1, &buffers[i][1]);
		}
		glBindVertexArray(buffers[i][0]);
		glBindBuffer(GL_ARRAY_BUFFER, buffers[i][1]);

		size_t totalCharCount = 0;
		for(size_t t = 0; t < TEXTURE_LIMIT && mapCounter2 < map.size(); t++, mapIt2++, mapCounter2++) { // <- ¯\_(ツ)_/¯
			totalCharCount += mapIt2->second.size();
		}
		if(!totalCharCount) continue;

		Vertex *verts = new Vertex[4 * totalCharCount];
		size_t vert_counter = 0;

		for(size_t t = 0; t < TEXTURE_LIMIT && mapCounter < map.size(); t++, mapIt++, mapCounter++) { // <- ¯\_(ツ)_/¯ x2

			for(auto &cp : mapIt->second) {
				verts[vert_counter * 4 + 0] = Vertex(cp.x, cp.y, t, 0);
				verts[vert_counter * 4 + 1] = Vertex(cp.x, cp.y - cp.h, t, 0);
				verts[vert_counter * 4 + 2] = Vertex(cp.x + cp.w, cp.y - cp.h, t, 0);
				verts[vert_counter * 4 + 3] = Vertex(cp.x + cp.w, cp.y, t, ~0);
				vert_counter++;
			}
		}
		if(!repos) glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * totalCharCount * 4, verts, GL_DYNAMIC_DRAW);
		else glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertex) * totalCharCount * 4, verts);
		delete[]verts;

		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void *>(0));
		glVertexAttribIPointer(1, 1, GL_INT, sizeof(Vertex), reinterpret_cast<void *>(sizeof(float) * 2));
		glVertexAttribIPointer(2, 1, GL_INT, sizeof(Vertex), reinterpret_cast<void *>(sizeof(float) * 2 + sizeof(int)));

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
	}


}


void TextField::text(std::shared_ptr<Font>f, const wchar_t *ndata) {
	font = f;
	size_t nLen = wcslen(ndata);
	if(data && nLen > dLen)
		data = reinterpret_cast<wchar_t *>(realloc(data, nLen + 1));
	else if(!data) data = new wchar_t[nLen + 1];
	dLen = nLen;
	wcscpy_s(data, dLen + 1, ndata);

	buildCharmap();
	assignTextures();

	setPositions();

	setBuffers();
}


// tmp
void TextField::repos(int w, int lh, float s) {
	width = w;
	lineHeight = lh;
	scale = s;

	map.clear();
	buildCharmap();
	assignTextures();

	setPositions();

	setBuffers(true);
}