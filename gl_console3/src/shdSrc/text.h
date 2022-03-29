#pragma once

namespace shdSrc {
	const char *vertex = R"(
		#version 430 core
		layout(location=0) in vec2 pos;
		layout(location=1) in int tex;
		layout(location=2) in int color;
		flat out ivec2 tc;
		out vec2 texCoord;
		layout(location=0)uniform mat4 projection = mat4(1);
		layout(location=1)uniform vec2 translate = vec2(0);
		void main() {
			tc = ivec2(tex, color);
			gl_Position = projection * vec4(pos + translate, -5, 1);
			switch(gl_VertexID % 4) {
				case 0: texCoord = vec2(0, 0); break;
				case 1: texCoord = vec2(0, 1); break;
				case 2: texCoord = vec2(1, 1); break;
				case 3: texCoord = vec2(1, 0); break;
			}
		}
	)";

	const char *fragment = R"(
		#version 430 core
		flat in ivec2 tc;
		in vec2 texCoord;
		out vec4 color;
		layout(location=20)uniform sampler2D sampler[%i];
		#define c(x) (((tc.y >> x) & 0xFF) / 255.f)
		void main() {
			color = vec4(c(24), c(16), c(8), texture(sampler[tc.x], texCoord).r * c(0));
			//color = vec4(texCoord, 0, 1);

			//color = vec4(texCoord, 0, 1);
		}
	)";

}