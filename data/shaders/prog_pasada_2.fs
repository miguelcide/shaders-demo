#version 330 core
out vec3 col;

uniform bool blinn = false;
uniform bool toon = false;
uniform bool bayer = false;

uniform sampler2D gAlbedo;
uniform sampler2D gNormals;
uniform sampler2D gWorldPos;
uniform sampler2D bayerT;

uniform vec3 camera;

uniform vec3 luz;
uniform vec3 colorLuz;
uniform vec4 coeficientes;

uniform float grosorBorde;
uniform vec3 colorBorde;

uniform uint nColoresD = 4u;
uniform uint nColoresS = 2u;

uniform vec2 resolution = vec2(800, 600);

void main() {
	vec2 fragCoord = gl_FragCoord.xy / resolution;
	vec3 nn = texture(gNormals, fragCoord).rgb;
	vec3 vv = normalize(camera - texture(gWorldPos, fragCoord).rgb);

	if (length(nn) == 0)
		discard;
	else if (dot(vv, nn) < grosorBorde) {
		col = colorBorde;
		return;
	}
	//Aqui va sobel, por ahora solo con las normales de gNormals
	//TODO: Hacer que el buffer de profundidad sea una textura para poder samplearla
	//gNormals se muestrea con gl_FragCoord.xy (+-1) / resolution


	float difusa = max(dot(luz, nn), 0);

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

	float ilu;
	if (toon) {
		float iluDifusa = floor(difusa * nColoresD) / (nColoresD - 1u);
		float iluSpecular = floor(specular * nColoresS) / (nColoresS - 1u);

		if (bayer) {
			float edge = texture(bayerT, gl_FragCoord.xy / 16.).r;

			float delta = difusa - iluDifusa;
			iluDifusa += (step(edge, delta) - step(delta, -edge)) / (nColoresD - 1u);

			delta = specular - iluSpecular;
			iluSpecular += (step(edge, delta) - step(delta, -edge)) / (nColoresS - 1u);
		}

		ilu = coeficientes.x + iluDifusa * coeficientes.y + iluSpecular * coeficientes.z;
	}
	else {
		ilu = coeficientes.x
			+ coeficientes.y * difusa
			+ coeficientes.z * specular;
	}

	col = colorLuz
		* ilu
		* texture(gAlbedo, fragCoord).rgb;
}
