#include <GpO.h>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

void init_imgui(GLFWwindow* window) 
{
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330 core");
}


void terminate_imgui(void) 
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}


void imgui_newframe(void) 
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}


void imgui_renderframe(void) 
{
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}


void imgui_renderShaderSelect(bool* useBlinn, bool* useToon, bool* useDither, bool* useHatching, bool* useSobelTex,
								bool* useSobelNorm, unsigned int* nColoresD, unsigned int* nColoresS, float* ditherScale, bool* improved_border)
{
	static int sel = 0, border_det = 0;
	static bool sTex = false, sNorm = true;

	if (ImGui::CollapsingHeader("Select shader", ImGuiTreeNodeFlags_DefaultOpen)) 
	{
		ImGui::RadioButton("Phong", &sel, 0); ImGui::SameLine();
		ImGui::RadioButton("Blinn", &sel, 1);
		ImGui::RadioButton("Border Detection", &border_det, 0); ImGui::SameLine();
		ImGui::RadioButton("Improved Border Detection", &border_det, 1);
	
		if(border_det){
			ImGui::Checkbox("Sobel sobre Albedo", &sTex); ImGui::SameLine();
			ImGui::Checkbox("Detección sobre normales", &sNorm);
			*useSobelTex = sTex;
			*useSobelNorm = sNorm;
		} else {
			*useSobelTex = false;
			*useSobelNorm = false;
		}
		
		ImGui::Checkbox("Toon-shading", useToon);
		
		if (*useToon) {
			ImGui::SameLine();
			ImGui::Checkbox("Dithering", useDither);
			if (*useDither)
				ImGui::SliderFloat("Dither texture scale", ditherScale, 1.0f, 16.0f, "%.0f");
			const unsigned int one = 1;
			ImGui::InputScalar("Nº tones (diffuse)", ImGuiDataType_U32, nColoresD, &one, NULL, "%u");
			ImGui::InputScalar("Nº tones (specular)", ImGuiDataType_U32, nColoresS, &one, NULL, "%u");
		}

		ImGui::Checkbox("Hatching", useHatching);
	}

	if (sel != *useBlinn)
		*useBlinn = sel;

	if (border_det != *improved_border)
		*improved_border = border_det;
}


bool imgui_renderSceneSelect(int* nScene, bool* useTextures) 
{
	static int sel = 0;

	if (ImGui::CollapsingHeader("Select scene", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::RadioButton("Isla", &sel, 0); ImGui::SameLine();
		ImGui::RadioButton("Torre", &sel, 1);
		ImGui::Checkbox("Show textures", useTextures);
	}

	if (sel != *nScene) {
		*nScene = sel;
		return true;
	}
	return false;
}

bool imgui_renderCameraPos(float* d, float* az, float* el) 
{
	bool res = false;

	if (ImGui::CollapsingHeader("Camera position", ImGuiTreeNodeFlags_DefaultOpen)) {
		res |= ImGui::SliderFloat("Distance##camara", d, 2.0f, 16.0f, "%.3f", ImGuiSliderFlags_NoRoundToFormat);
		res |= ImGui::SliderAngle("Azimuth##camara", az, 0.0f, 360.0f, "%.1f deg", ImGuiSliderFlags_NoRoundToFormat);
		res |= ImGui::SliderAngle("Elevation##camara", el, -89.999f, 89.999f, "%.1f deg", ImGuiSliderFlags_NoRoundToFormat);
	}

	return res;
}

bool imgui_renderLightVec(float* az, float* el) 
{
	bool res = false;

	if (ImGui::CollapsingHeader("Global light vector")) {
		res |= ImGui::SliderAngle("Azimuth##luzglobal", az, 0.0f, 360.0f, "%.1f deg", ImGuiSliderFlags_NoRoundToFormat);
		res |= ImGui::SliderAngle("Elevation##luzglobal", el, -89.999f, 89.999f, "%.1f deg", ImGuiSliderFlags_NoRoundToFormat);
	}

	return res;
}

void imgui_renderLightColor(vec3* lightColor, vec3* bgColor) 
{
	if (ImGui::CollapsingHeader("Scene colors")) {
		ImGui::ColorPicker3("Light color", &lightColor->r, //Esto es una autentica guarrada que puede explotar en cualquier momento
							ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_DisplayRGB);
		ImGui::ColorPicker3("Sky color", &bgColor->r,
							ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_DisplayRGB);
	}
}

void imgui_renderCoefficients(vec4* coeficientes) 
{
	if (ImGui::CollapsingHeader("Material coefficients")) 
	{
		ImGui::SliderFloat("Ambient##coeficientes", &(coeficientes->x), 0.0f, 1.0f, "%.2f");
		ImGui::SliderFloat("Diffuse##coeficientes", &(coeficientes->y), 0.0f, 1.0f, "%.2f");
		ImGui::SliderFloat("Specular##coeficientes", &(coeficientes->z), 0.0f, 1.0f, "%.2f");
		ImGui::SliderFloat("Shininess##coeficientes", &(coeficientes->w), 1.0f, 100.0f, "%.0f");
	}
}

void imgui_renderBorderSettings(vec3* color, float* grosorBorde)
{
	if (ImGui::CollapsingHeader("Border Detection")) 
	{
		ImGui::SliderFloat("Border Thickness##borderColor", grosorBorde, 0.0f, 1.0f, "%.2f");
			
		ImGui::ColorPicker3("##borderColor", &color->r, //Esto puede explotar en cualquier momento
							ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_DisplayRGB);
	}
}

void imgui_renderImprovedBorderSettings(vec3* color, float* tex_treshold, float* norm_treshold) 
{

	if (ImGui::CollapsingHeader("Improved Border Detection")) 
	{
		ImGui::SliderFloat("Sobel Sensibility##borderColor", tex_treshold, 0.0f, 2.0f, "%.2f");
		ImGui::SliderFloat("Normal Threshold##borderColor", norm_treshold, 0.0f, 2.0f, "%.2f");
		
		ImGui::ColorPicker3("##borderColor", &color->r, //Esto puede explotar en cualquier momento
							ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_DisplayRGB);
	}
}

