#version 330 core
in vec3 norm;
in vec2 UV;
in vec3 vision;
out vec3 col;

uniform bool blinn = false;

uniform sampler2D unit;

uniform vec3 luz;
uniform vec3 colorLuz;
uniform vec4 coeficientes;

uniform float grosorBorde;
uniform vec3 colorBorde;

void main() {
	vec3 nn = normalize(norm);
	vec3 vv = normalize(vision);

	if (dot(vv, nn) < grosorBorde) {
		col = colorBorde;
		return;
	}

	float difusa = dot(luz, nn);
	if (difusa < 0) difusa = 0;

	float specular;
	if (blinn) {
		vec3 halfAngle = normalize(luz + vv);
		specular = dot(nn, halfAngle);
	}
	else {
		vec3 r = reflect(-luz, nn);
		specular = dot(r, vv);
	}
	specular = (specular > 0 ? pow(specular, coeficientes.w) : 0);

	float ilu = coeficientes.x
		+ coeficientes.y * difusa
		+ coeficientes.z * specular;

	col = colorLuz
		* ilu
		* texture(unit, UV).rgb;
}
