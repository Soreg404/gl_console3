#pragma once

#include <unordered_map>
#include <map>
#include <list>
#include <array>

typedef __int32 int32_t;
typedef unsigned __int32 uint32_t;


struct CharIndexed {
	GLuint tex = 0;
	int w = 0, h = 0, l = 0, t = 0, a = 0;
};

struct CharBox {
	float x = 0, y = 0, w = 0, h = 0;
};
typedef std::map<wchar_t, std::vector<CharBox>> charmap;

struct Library {
	Library();
	~Library();
	inline struct FT_LibraryRec_ *get() { return library; }
	inline GLuint EBO() { return m_EBO; }
	inline void bindShd() const { glUseProgram(m_shd); }
	inline GLint texLimit() const { return m_texLimit; }

private:
	struct FT_LibraryRec_ *library = nullptr;
	GLuint m_EBO = 0, m_shd = 0;
	GLint m_texLimit = 0;
};

class Font {
	friend class TextField;
public:

	Font(std::shared_ptr<Library>);
	~Font();

	void load(const char *path);
	void setSize(uint32_t w, uint32_t h);

	inline struct { uint32_t w, h; } getSize() const { return { w, h }; }

private:
	uint32_t w = 0, h = 0;

	std::unordered_map<wchar_t, CharIndexed> index;

	struct FT_FaceRec_ *face = nullptr;
	std::shared_ptr<Library> lib;
};


struct BufferChunk {
	GLuint VAO = 0, VBO = 0;
	size_t total_count = 0;
	charmap::iterator head;
};

class TextField {

public:

	~TextField();

	void draw(float x, float y);

	void text(std::shared_ptr<Font> font, const wchar_t *data);
	void repos(int width, int lineHeight, float scale = 1.f);

private:
	uint32_t status = 0;

	std::vector<BufferChunk> buffers;
	size_t nBuffs = 0; // TODO
	wchar_t *data = nullptr;
	size_t dLen = 0;

	int width = 200, lineHeight = 40;
	float scale = 1.f;

	int lines = 0;
	charmap map;

	bool isClear = true;
	void clear();

	void buildCharmap();
	void assignTextures();
	void setPositions();
	void setBuffers();

	std::shared_ptr<Font> font;


public:
	inline int getWidth() const { return width; }
	inline int getLineHeight() const { return lineHeight; }
	inline float getScale() const { return scale; }
	inline int getLines() const { return lines; }

};


struct Color {
	float r = 0, g = 0, b = 0, a = 1;
	Color(float r, float g, float b, float a);
	Color(int r, int g, int b, int a);
	Color(float all);
	Color(int all);
};