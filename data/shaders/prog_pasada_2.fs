#version 330 core
out vec3 col;

uniform bool blinn = false;
uniform bool toon = false;
uniform bool bayer = false;
uniform bool hatching = false;
uniform bool useSobelTex = false;
uniform bool useSobelNorm = false;

uniform sampler2D gAlbedo;
uniform sampler2D gDepth;
uniform sampler2D gNormals;
uniform sampler2D gWorldPos;
uniform sampler2D bayerT;
uniform sampler2D hatchT;

uniform vec3 camera;

uniform vec3 luz;
uniform vec3 colorLuz;
uniform vec4 coeficientes;

uniform float grosorBorde;
uniform float sobelBorde;
uniform float normalBorde;
uniform vec3 colorBorde;

uniform uint nColoresD = 4u;
uniform uint nColoresS = 2u;

uniform float ditherScale = 1.0f;

uniform vec2 resolution = vec2(800, 600);


float sobel(sampler2D gBuffer) {
	vec2 texelSize = 1.0 / resolution;
	vec3 sobelX, sobelY;

	sobelX = abs(texture(gBuffer, (gl_FragCoord.xy + vec2(-1, -1)) * texelSize).rgb * -1.0 +
				texture(gBuffer, (gl_FragCoord.xy + vec2( 1, -1)) * texelSize).rgb *  1.0 +
				texture(gBuffer, (gl_FragCoord.xy + vec2(-1,  0)) * texelSize).rgb * -2.0 +
				texture(gBuffer, (gl_FragCoord.xy + vec2( 1,  0)) * texelSize).rgb *  2.0 +
				texture(gBuffer, (gl_FragCoord.xy + vec2(-1,  1)) * texelSize).rgb * -1.0 +
				texture(gBuffer, (gl_FragCoord.xy + vec2( 1,  1)) * texelSize).rgb *  1.0);

	sobelY = abs(texture(gBuffer, (gl_FragCoord.xy + vec2(-1, -1)) * texelSize).rgb * -1.0 +
				texture(gBuffer, (gl_FragCoord.xy + vec2(-1,  1)) * texelSize).rgb *  1.0 +
				texture(gBuffer, (gl_FragCoord.xy + vec2( 0, -1)) * texelSize).rgb * -2.0 +
				texture(gBuffer, (gl_FragCoord.xy + vec2( 0,  1)) * texelSize).rgb *  2.0 +
				texture(gBuffer, (gl_FragCoord.xy + vec2( 1, -1)) * texelSize).rgb * -1.0 +
				texture(gBuffer, (gl_FragCoord.xy + vec2( 1,  1)) * texelSize).rgb *  1.0);

	return length(sqrt(sobelX*sobelX + sobelY*sobelY));
}

float normal_edge_detection(sampler2D gBuffer){
	vec2 texelSize = 1.0 / resolution;

	vec3 center = texture(gBuffer, (gl_FragCoord.xy) * texelSize).rgb;
	vec3 left = texture(gBuffer, (gl_FragCoord.xy + vec2(0, -1)) * texelSize).rgb;
	vec3 right = texture(gBuffer, (gl_FragCoord.xy + vec2(0, 1)) * texelSize).rgb;
	vec3 top = texture(gBuffer, (gl_FragCoord.xy + vec2(1, 0)) * texelSize).rgb;
	vec3 bottom = texture(gBuffer, (gl_FragCoord.xy + vec2(-1, 0)) * texelSize).rgb;
	
	vec3 res = abs(left - center) + abs(right - center) + abs(top - center) + abs(bottom - center);
	return length(res);
}


void main() {
	vec2 fragCoord = gl_FragCoord.xy / resolution; 
	vec3 nn = texture(gNormals, fragCoord).rgb;
	vec3 vv = normalize(camera - texture(gWorldPos, fragCoord).rgb);

	if (length(nn) == 0)
		discard;

	// Edge detection
	if(!useSobelTex && !useSobelNorm && dot(vv, nn) < grosorBorde){
		col = colorBorde;
		return;
	}


	// Improved edge detection
	float magnitude = 0, normalMagnitude = 0;
	if (useSobelTex)
		magnitude = sobel(gAlbedo);

	if (useSobelNorm)
		normalMagnitude = normal_edge_detection(gNormals);

	if (normalMagnitude >= normalBorde || magnitude >= sobelBorde) {
		col = colorBorde;
		return;
	}


	// Light model (Phong/Blinn)
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


	// Toon-Shading
	if (toon) {
		float iluDifusa = floor(difusa * nColoresD) / (nColoresD - 1u);
		float iluSpecular = floor(specular * nColoresS) / (nColoresS - 1u);

		// Dithering
		if (bayer) {
			float edge = texture(bayerT, gl_FragCoord.xy / (16. * ditherScale)).r;

			float delta = difusa - iluDifusa;
			iluDifusa += (step(edge, delta) - step(delta, -edge)) / (nColoresD - 1u);

			delta = specular - iluSpecular;
			iluSpecular += (step(edge, delta) - step(delta, -edge)) / (nColoresS - 1u);
		}

		difusa = iluDifusa;
		specular = iluSpecular;
	}


	// Hatching
	if (hatching) {
		vec2 texCoord = gl_FragCoord.xy / 256.;
		texCoord.x = fract(texCoord.x) * 0.25;
		float sample0 = texture(hatchT, texCoord).r;
		float sample1 = texture(hatchT, texCoord + 0.25).r;
		float sample2 = texture(hatchT, texCoord + 0.5).r;
		float sample3 = texture(hatchT, texCoord + 0.75).r;

		//https://math.stackexchange.com/questions/544559/is-there-any-equation-for-triangle
		difusa = sample0 * clamp(1.5 - 5. * difusa, 0., 1.)
					+ sample1 * max(5.*(difusa - 0.3) * sign(0.3 - difusa) + 5.*0.3 - 0.5, 0.)
					+ sample2 * max(5.*(difusa - 0.5) * sign(0.5 - difusa) + 5.*0.5 - 1.5, 0.)
					+ sample3 * max(5.*(difusa - 0.7) * sign(0.7 - difusa) + 5.*0.7 - 2.5, 0.)
					+ clamp(5 * difusa - 3.5, 0., 1.);
		specular = sample0 * clamp(1.5 - 5. * specular, 0., 1.)
					+ sample1 * max(5.*(specular - 0.3) * sign(0.3 - specular) + 5.*0.3 - 0.5, 0.)
					+ sample2 * max(5.*(specular - 0.5) * sign(0.5 - specular) + 5.*0.5 - 1.5, 0.)
					+ sample3 * max(5.*(specular - 0.7) * sign(0.7 - specular) + 5.*0.7 - 2.5, 0.)
					+ clamp(5 * specular - 3.5, 0., 1.);
	}

	float ilu = coeficientes.x
				+ coeficientes.y * difusa
				+ coeficientes.z * specular;

	col = colorLuz
		* ilu
		* texture(gAlbedo, fragCoord).rgb;
}
