#version 330 core
layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec3 normal;
out vec3 norm;
out vec2 UV;
out vec3 vision;

uniform mat4 M;
uniform mat4 PV;
uniform vec3 camera;

void main() {
  UV = uv;

  vec4 globalPos = M * vec4(pos, 1);
  gl_Position = PV * globalPos;
  vision = camera - vec3(globalPos);

  mat3 M_adj = mat3(transpose(inverse(M)));
  norm = M_adj * normal;
}
