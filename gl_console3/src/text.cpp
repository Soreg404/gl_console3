#include <ft2build.h>
#include FT_FREETYPE_H

#include "console.h"
#include "shdSrc/text.h"

char TMPSTR[1000] = { 0 };

const uint32_t INDICE_COUNT = 4000;

#pragma pack(push, 1)
struct Vertex {
	float x, y;
	int tex, color;
	Vertex(float x = 0, float y = 0, int tex = 0, int color = 0): x(x), y(y), tex(tex), color(color) {}
};

struct Indice {
	std::array<uint32_t, 6> v = { 0 };
	Indice() = default;
	Indice(uint32_t b) {
		uint32_t t[] = { b + 0, b + 1, b + 3, b + 1, b + 2, b + 3 };
		memcpy_s(v.data(), 6 * sizeof(uint32_t), t, 6 * sizeof(uint32_t));
	}
};
#pragma pack(pop)


void Console::drawText() {

	int x = layout.bar_left, y = layout.bar_bottom;
	int vW = wW - x - layout.bar_right, vH = wH - y - layout.toolbar;
	glViewport(x, y, vW, vH);

	GLERR;

	glUseProgram(4);

	GLERR;

	glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(glm::ortho(0.f, static_cast<float>(vW), 0.f, static_cast<float>(vH), 1.f, 10.f)));


	GLERR;

	int yPos = layout.input_bg + font->getSize().h * .5;
	for(auto it = logBuffer.rbegin(); it != logBuffer.rend(); it++) {
		if(yPos - it->getLineHeight() > vH) break;
		yPos += it->getLineHeight() * it->getLines();
		it->draw(0, static_cast<float>(yPos));
	}

	input.draw(10, 16 + (layout.input_bg - 16) / 2);

	GLERR;

	glViewport(0, 0, wW, wH);

}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


Library::Library() {
	FT_Init_FreeType(&library);


	glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &m_texLimit);
	LOG("texture units: %i", m_texLimit);

	sprintf_s(TMPSTR, 1000, shdSrc::fragment, m_texLimit);
	m_shd = prepShd(shdSrc::vertex, TMPSTR);

	glUseProgram(m_shd);
	for(int i = 0; i < m_texLimit; i++) glUniform1iv(20 + i, 1, &i);

	glGenBuffers(1, &m_EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
	Indice *inds = new Indice[INDICE_COUNT];
	for(uint32_t i = 0, vbuffCounter = 0; i < INDICE_COUNT; i++, vbuffCounter += 4)
		inds[i] = { vbuffCounter };
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, INDICE_COUNT * 6 * sizeof(uint32_t), inds, GL_STATIC_DRAW);
	delete[]inds;
}

Library::~Library() { FT_Done_FreeType(library); }

Font::Font(std::shared_ptr<Library> l): lib(l) {}

Font::~Font() { FT_Done_Face(face); }

void Font::load(const char *path) {
	FT_Done_Face(face);
	if(!FT_New_Face(lib->get(), path, 0, &face)) {
		index.clear();
	} else LOG("an error occured while loading font face");
}

void Font::setSize(uint32_t _w, uint32_t _h) {
	FT_Set_Pixel_Sizes(face, _w, _h);
	w = _w; h = _h;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TextField::~TextField() {

}

size_t dc = 0;
void TextField::draw(float x, float y) {
	if(!font || !data || isClear) return;

	font->lib->bindShd();

	glUniform2f(1, x, y);

	auto mapIt = map.begin();
	size_t mapCounter = 0;

	GLint tLim = font->lib->texLimit();

	for(size_t i = 0; i < buffers.size(); i++) {

		glBindVertexArray(buffers[i].VAO);

		// bind textures
		for(GLint t = 0; t < tLim && mapIt != map.end(); t++, mapIt++) {
			glActiveTexture(GL_TEXTURE0 + t);
			glBindTexture(GL_TEXTURE_2D, font->index[mapIt->first].tex);
		}

		//LOG("DRAW CALL %zu", dc);
		dc++;
		//glDrawArrays(GL_QUADS, 0, 600);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, font->lib->EBO());
		glDrawElements(GL_TRIANGLES, buffers[i].total_count * 6, GL_UNSIGNED_INT, (void *)0);
	}

}


void TextField::buildCharmap() { for(size_t i = 0; data[i]; i++) map[data[i]]; }

CharIndexed loadCharTex(FT_Face &face, char32_t c) {
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
	lines = 1;

	// tmp
	for(auto &m : map) m.second.clear();

	for(size_t i = 0; c = data[i]; i++) {
		auto found = font->index.find(c);
		if(found == font->index.end()) continue;
		CharIndexed ci = found->second;

		CharBox pos;

		if((advance + ci.w) * scale > width) { lines++; advance = 0; }

		pos.x = (advance + ci.l) * scale;
		pos.y = -(lines * lineHeight) + ci.t * scale;
		pos.w = ci.w * scale;
		pos.h = ci.h * scale;

		map[c].push_back(pos);

		advance += (ci.a >> 6);
	}
}

size_t countTotalChars(const charmap &map, charmap::iterator it, GLint limit) {
	size_t ret = 0;
	for(GLint t = 0; t < limit && it != map.end(); t++, it++) ret += it->second.size();
	return ret;
}

void TextField::setBuffers() {
	
	if(!map.size()) { LOG("attempted setBuffers on empty map"); return; }
	charmap::iterator mapHead = map.begin();

	GLint tLim = font->lib->texLimit();

	if(isClear) buffers = std::vector<BufferChunk>(map.size() / tLim + 1);

	for(auto &buff : buffers) {

		buff.head = mapHead;

		// \/\/\/ errors \/\/\/
		if(isClear) {
			glGenVertexArrays(1, &buff.VAO);
			glGenBuffers(1, &buff.VBO);
		}
		glBindVertexArray(buff.VAO);
		glBindBuffer(GL_ARRAY_BUFFER, buff.VBO);


		if(!(buff.total_count = countTotalChars(map, mapHead, tLim))) continue;

		std::vector<std::array<Vertex, 4>> verts(buff.total_count);
		size_t vert_counter = 0;

		for(GLint t = 0; t < tLim && mapHead != map.end(); t++, mapHead++) {
			for(auto &cp : mapHead->second) {
				assert(vert_counter < buff.total_count);
				verts[vert_counter] = {
					Vertex(cp.x, cp.y, t, 0),
					Vertex(cp.x, cp.y - cp.h, t, 0),
					Vertex(cp.x + cp.w, cp.y - cp.h, t, 0),
					Vertex(cp.x + cp.w, cp.y, t, ~0)
				};
				vert_counter++;
			}
		}

		if(isClear) glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * buff.total_count * 4, verts.data(), GL_DYNAMIC_DRAW);
		else glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertex) * buff.total_count * 4, verts.data());
		verts.clear();

		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void *>(0));
		glVertexAttribIPointer(1, 1, GL_INT, sizeof(Vertex), reinterpret_cast<void *>(sizeof(float) * 2));
		glVertexAttribIPointer(2, 1, GL_INT, sizeof(Vertex), reinterpret_cast<void *>(sizeof(float) * 2 + sizeof(int)));

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
	}


}

void TextField::clear() {

	// tmp
	map.clear();
	
	isClear = true;
}

void TextField::repos(int w, int lh, float s) {
	width = w;
	lineHeight = lh;
	scale = s;

	if(!data || !dLen || !font) return;

	setPositions();

	setBuffers();

	isClear = false;
}

void TextField::text(std::shared_ptr<Font>f, const wchar_t *ndata) {
	if(!f || !ndata) {
		if(!f) font = nullptr;
		if(!ndata && data) delete[]data; data = nullptr;
		return;
	}
	font = f;
	size_t nLen = wcslen(ndata);
	if(data && nLen > dLen) {
		wchar_t *tmp = reinterpret_cast<wchar_t *>(realloc(data, (nLen + 1) * sizeof(wchar_t)));
		if(tmp) data = tmp;
		else {
			delete[]data;
			dLen = 0;
			return;
		}
	}
	else if(!data) data = new wchar_t[nLen + 1];
	dLen = nLen;
	wcscpy_s(data, dLen + 1, ndata);

	clear();

	buildCharmap();
	assignTextures();

}
