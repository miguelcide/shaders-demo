#version 330 core
layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;

out vec3 adjNorm;
out vec3 worldPos;
out vec2 UV;

uniform mat4 M;
uniform mat4 PV;

void main() {
  UV = uv;

  vec4 globalPos = M * vec4(pos, 1);
  worldPos = globalPos.xyz;
  gl_Position = PV * globalPos;

  mat3 M_adj = mat3(transpose(inverse(M)));
  adjNorm = M_adj * normal;
}
