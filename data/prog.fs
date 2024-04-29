#version 330 core
in vec3 norm;
in vec2 UV;
in vec3 vision;
out vec3 col;

uniform sampler2D unit;
uniform vec3 luz = vec3(1, 1, 0) / sqrt(2.0f);
uniform vec4 coeficientes = vec4(0.1, 0.6, 0.3, 16);

void main() {
	vec3 nn = normalize(norm);

	float difusa = dot(luz, nn);
	if (difusa < 0) difusa = 0;

	vec3 r = reflect(-luz, nn);
	float specular = dot(r, normalize(vision));
	specular = (specular > 0 ? pow(specular, coeficientes.w) : 0);

	float ilu = coeficientes.x
		+ coeficientes.y * difusa
		+ coeficientes.z * specular;

	col = vec3(1, 1, 1)
		* ilu
		* texture(unit, UV).rgb;
}
