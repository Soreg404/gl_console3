#pragma once

#include <unordered_map>
#include <list>
#include <array>

typedef __int32 int32_t;
typedef unsigned __int32 uint32_t;


struct CharIndexed {
	GLuint tex = 0;
	int w = 0, h = 0, l = 0, t = 0, a = 0;
};

struct CharMapped {
	float x = 0, y = 0, w = 0, h = 0;
};

// for every font face and char size
typedef std::unordered_map<wchar_t, CharIndexed> charindex;
// for every tex container
typedef std::unordered_map<wchar_t, std::list<CharMapped>> charmap;

struct Library {
	Library();
	~Library();
	inline struct FT_LibraryRec_ *get() { return library; }
	inline GLuint EBO() { return m_EBO; }

private:
	struct FT_LibraryRec_ *library = nullptr;
	GLuint m_EBO = 0;
};

class Font {
	friend class TextField;
public:

	Font(std::shared_ptr<Library>);
	~Font();

	void load(const char *path);
	void setSize(uint32_t w, uint32_t h);

private:
	uint32_t w, h;

	charindex index;

	struct FT_FaceRec_ *face = nullptr;
	std::shared_ptr<Library> lib;
};


class TextField {

public:

	~TextField();

	void draw();

	void text(std::shared_ptr<Font> font, const wchar_t *data);
	void repos(int width, int lineHeight, float scale = 1.f);

private:
	uint32_t status = 0;

	std::vector<std::array<GLuint, 2>> buffers;
	size_t nBuffs = 0; // TODO
	wchar_t *data = nullptr;
	size_t dLen = 0;

	int width = 200, lineHeight = 40;
	float scale = 1.f;

	int lines = 0;
	charmap map;

	void clearTextures();
	void clearBuffers();

	void buildCharmap();
	void assignTextures();
	void setPositions();
	void setBuffers(bool repos = false);

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