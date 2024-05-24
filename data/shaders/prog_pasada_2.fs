#version 330 core
out vec3 col;

uniform bool blinn = false;
uniform bool toon = false;
uniform bool bayer = false;
uniform bool useSobelTex = false;
uniform bool useSobelNorm = false;

uniform sampler2D gAlbedo;
uniform sampler2D gDepth;
uniform sampler2D gNormals;
uniform sampler2D gWorldPos;
uniform sampler2D bayerT;

uniform vec3 camera;

uniform vec3 luz;
uniform vec3 colorLuz;
uniform vec4 coeficientes;

uniform float grosorBorde;
uniform float normalBorde;
uniform vec3 colorBorde;

uniform uint nColoresD = 4u;
uniform uint nColoresS = 2u;

uniform vec2 resolution = vec2(800, 600);

float gaussianKernel[25] = float[](
    1.0, 4.0, 6.0, 4.0, 1.0,
    4.0, 16.0, 24.0, 16.0, 4.0,
    6.0, 24.0, 36.0, 24.0, 6.0,
    4.0, 16.0, 24.0, 16.0, 4.0,
    1.0, 4.0, 6.0, 4.0, 1.0
);

float vec3_to_avg(vec3 vector){
	return (vector.r + vector.g + vector.b) / 3.0f;
}

bool compare_vec(vec3 v1, vec3 v2, float threshold){
	return abs(vec3_to_avg(v1) - vec3_to_avg(v2)) > threshold;
}

void main() {
	
	vec2 fragCoord = gl_FragCoord.xy / resolution; //pos del pixel
	vec3 nn = texture(gNormals, fragCoord).rgb;
	vec3 vv = normalize(camera - texture(gWorldPos, fragCoord).rgb);
	float z = texture(gDepth, fragCoord).r;
	float depthThreshold = 0.5;

	if (length(nn) == 0)
		discard;
	// else if (dot(vv, nn) < grosorBorde) {		
	// 	col = colorBorde;
	// 	return;				
	// }

	//Aqui va sobel
	if (useSobelTex || useSobelNorm) {
		vec2 texelSize = 1.0 / resolution;
		float sobelX, sobelY;
		for(int i=0; i<3; i++){	
			sobelX = texture(gAlbedo, fragCoord + texelSize * vec2(-1, -1))[i] * -1.0 +
					texture(gAlbedo, fragCoord + texelSize * vec2( 1, -1))[i] *  1.0 +
					texture(gAlbedo, fragCoord + texelSize * vec2(-2,  0))[i] * -2.0 +
					texture(gAlbedo, fragCoord + texelSize * vec2( 2,  0))[i] *  2.0 +
					texture(gAlbedo, fragCoord + texelSize * vec2(-1,  1))[i] * -1.0 +
					texture(gAlbedo, fragCoord + texelSize * vec2( 1,  1))[i] *  1.0;

			sobelY = texture(gAlbedo, fragCoord + texelSize * vec2(-1, -1))[i] * -1.0 +
							texture(gAlbedo, fragCoord + texelSize * vec2(-1,  1))[i] *  1.0 +
							texture(gAlbedo, fragCoord + texelSize * vec2( 0, -2))[i] * -2.0 +
							texture(gAlbedo, fragCoord + texelSize * vec2( 0,  2))[i] *  2.0 +
							texture(gAlbedo, fragCoord + texelSize * vec2( 1, -1))[i] * -1.0 +
							texture(gAlbedo, fragCoord + texelSize * vec2( 1,  1))[i] *  1.0;
		}
		
		sobelX /= 3.0f;
		sobelY /= 3.0f;


		float magnitude = sqrt(sobelX * sobelX + sobelY * sobelY);

		vec2 topCoord = fragCoord + vec2(0.0, texelSize.y);
		vec2 rightCoord = fragCoord + vec2(texelSize.x, 0.0);
		vec2 leftCoord = fragCoord - vec2(texelSize.x, 0.0);
		vec2 bottomCoord = fragCoord - vec2(0.0, texelSize.y);

		// Obtener las normales de los píxeles vecinos
		vec3 topNormal = texture(gNormals, topCoord).rgb;
		vec3 rightNormal = texture(gNormals, rightCoord).rgb;
		vec3 leftNormal = texture(gNormals, leftCoord).rgb;
		vec3 bottomNormal = texture(gNormals, bottomCoord).rgb;
		
		// Si las normales de dos pixeles vecinos son diferentes, pintamos el borde
		 
		if(useSobelTex ){
			if (magnitude >= grosorBorde) {
				col = colorBorde;
				return;
			}
		}
		else if(useSobelNorm){
			if (compare_vec(topNormal, bottomNormal, normalBorde) && compare_vec(leftNormal, rightNormal, normalBorde)){
				col = colorBorde;
					return;
			}
		}
		else if (useSobelTex && useSobelNorm){
			if ((magnitude >= grosorBorde) && (compare_vec(topNormal, bottomNormal, normalBorde) && compare_vec(leftNormal, rightNormal, normalBorde))){
				col = colorBorde;
					return;
			}
		}
		

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
