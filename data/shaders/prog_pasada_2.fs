#version 330 core
out vec3 col;

uniform bool blinn = false;
uniform bool toon = false;
uniform bool bayer = false;
uniform bool hatching = false;
uniform bool useSobelTex = false;
uniform bool useSobelNorm = false;
uniform bool useSobelDepth = false;

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
uniform float normalBorde;
uniform float profundidadBorde;
uniform vec3 colorBorde;

uniform uint nColoresD = 4u;
uniform uint nColoresS = 2u;

uniform float ditherScale = 1.0f;

uniform vec2 resolution = vec2(800, 600);

const float gaussianKernel[25] = float[] (
    1.0, 4.0, 6.0, 4.0, 1.0,
    4.0, 16.0, 24.0, 16.0, 4.0,
    6.0, 24.0, 36.0, 24.0, 6.0,
    4.0, 16.0, 24.0, 16.0, 4.0,
    1.0, 4.0, 6.0, 4.0, 1.0
);

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

float depth_edge_detection(sampler2D gBuffer) {
    vec2 texelSize = 1.0 / resolution;

    float center = texture(gBuffer, gl_FragCoord.xy * texelSize).r;
    float left = texture(gBuffer, (gl_FragCoord.xy + vec2(0, -1)) * texelSize).r;
    float right = texture(gBuffer, (gl_FragCoord.xy + vec2(0, 1)) * texelSize).r;
    float top = texture(gBuffer, (gl_FragCoord.xy + vec2(1, 0)) * texelSize).r;
    float bottom = texture(gBuffer, (gl_FragCoord.xy + vec2(-1, 0)) * texelSize).r;

    float res = abs(left - center) + abs(right - center) + abs(top - center) + abs(bottom - center);
    return res;
}


void main() {
	vec2 fragCoord = gl_FragCoord.xy / resolution; //pos del pixel
	vec3 nn = texture(gNormals, fragCoord).rgb;
	vec3 vv = normalize(camera - texture(gWorldPos, fragCoord).rgb);
	float z = texture(gDepth, fragCoord).r;

	if (length(nn) == 0)
		discard;
	// else if (dot(vv, nn) < grosorBorde) {
	// 	col = colorBorde;
	// 	return;
	// }

	float magnitude = 0, normalMagnitude = 0, depthMagnitude = 0;
	if (useSobelTex)
		// magnitude = max(magnitude, clamp(sobel(gAlbedo) / 16, 0.0, 1.0));
		magnitude = max(magnitude, sobel(gAlbedo));
	if (useSobelNorm)
		// magnitude = max(magnitude, clamp(sobel(gNormals) / 64, 0.0, 1.0)); //norm con los cuadrados para bajar el borde de las normales
		//magnitude = max(magnitude, sobel(gNormals) / 8);
		normalMagnitude = normal_edge_detection(gNormals);
	if (useSobelDepth)
		// magnitude = max(magnitude, clamp(sobel(gDepth)/ 16, 0.0, 1.0));
		// magnitude = max(magnitude, sobel(gDepth));
		depthMagnitude = depth_edge_detection(gDepth);
		
	if (normalMagnitude >= normalBorde || magnitude >= grosorBorde || depthMagnitude >= profundidadBorde)	{
		col = colorBorde;
		return;
	}


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

	if (toon) {
		float iluDifusa = floor(difusa * nColoresD) / (nColoresD - 1u);
		float iluSpecular = floor(specular * nColoresS) / (nColoresS - 1u);

		if (bayer) {
			float edge = texture(bayerT, gl_FragCoord.xy * ditherScale / 16.).r;

			float delta = difusa - iluDifusa;
			iluDifusa += (step(edge, delta) - step(delta, -edge)) / (nColoresD - 1u);

			delta = specular - iluSpecular;
			iluSpecular += (step(edge, delta) - step(delta, -edge)) / (nColoresS - 1u);
		}

		difusa = iluDifusa;
		specular = iluSpecular;
	}

	if (hatching) {
		vec2 texCoord = gl_FragCoord.xy / 256.;
		texCoord.x = fract(texCoord.x) * 0.25;
		float sample0 = texture(hatchT, texCoord).r;
		float sample1 = texture(hatchT, texCoord + 0.25).r;
		float sample2 = texture(hatchT, texCoord + 0.5).r;
		float sample3 = texture(hatchT, texCoord + 0.75).r;

		//https://math.stackexchange.com/questions/544559/is-there-any-equation-for-triangle
		difusa = sample0 * clamp(1.5 - 4. * difusa, 0., 1.)
					+ sample1 * max(4.*(difusa - 0.375) * sign(0.375 - difusa) + 4.*0.375 - 0.5, 0.)
					+ sample2 * max(4.*(difusa - 0.625) * sign(0.625 - difusa) + 4.*0.625 - 1.5, 0.)
					+ sample3 * clamp(4 * difusa - 2.5, 0., 1.);
		specular = sample0 * clamp(1.5 - 4. * specular, 0., 1.)
					+ sample1 * max(4.*(specular - 0.375) * sign(0.375 - specular) + 4.*0.375 - 0.5, 0.)
					+ sample2 * max(4.*(specular - 0.625) * sign(0.625 - specular) + 4.*0.625 - 1.5, 0.)
					+ sample3 * clamp(4 * specular - 2.5, 0., 1.);
	}

	float ilu = coeficientes.x
				+ coeficientes.y * difusa
				+ coeficientes.z * specular;

	col = colorLuz
		* ilu
		* texture(gAlbedo, fragCoord).rgb;
}
