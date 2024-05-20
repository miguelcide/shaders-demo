#version 330 core
in vec3 adjNorm;
in vec3 worldPos;
in vec2 UV;

layout(location=0) out vec3 gAlbedo;
layout(location=1) out vec3 gNormals;
layout(location=2) out vec3 gWorldPos;

uniform sampler2D baseTexture;

void main() {
	gNormals = normalize(adjNorm);
	gAlbedo = texture(baseTexture, UV).rgb;
	gWorldPos = worldPos;
}
