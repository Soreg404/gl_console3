#pragma once

#include <unordered_map>
#include <list>
#include <array>

typedef __int32 int32_t;
typedef unsigned __int32 uint32_t;


struct CharInfo {
	GLuint tex = 0;
	int w = 0, h = 0, l = 0, t = 0, a = 0;
	std::list<glm::vec4> pos;
};


// for every font face and char size
typedef std::unordered_map<wchar_t, CharInfo> charindex;
// for every tex container
typedef std::unordered_map<wchar_t, CharInfo> charmap;

struct Font {
	Font() = default;
	void set(struct FT_LibraryRec_ *library, const char *path);
	struct FT_FaceRec_ *face = nullptr;
	charindex index;
};

//struct Library {
//	Library();
//	~Library();
//	struct FT_LibraryRec_ *library = nullptr;
//	std::unique_ptr<Font> makeFont();
//};


struct TextField {

	int width = 200, lineHeight = 40;
	float scale = 1.f;

	std::vector<GLuint> buffers;
	charmap map;

	uint32_t lines = 0;

	void set(Font &face, const wchar_t *data);
private:
	wchar_t *data = nullptr;
};


struct Color {
	float r = 0, g = 0, b = 0, a = 1;
	Color(float r, float g, float b, float a);
	Color(int r, int g, int b, int a);
	Color(float all);
	Color(int all);
};