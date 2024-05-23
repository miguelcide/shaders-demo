#include <GpO.h>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

void init_imgui(GLFWwindow* window) {
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

void terminate_imgui(void) {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void imgui_newframe(void) {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

bool imgui_renderShaderSelect(bool* useBlinn, bool* useToon, bool* useDither, bool* useSobel, unsigned int* nColoresD, unsigned int* nColoresS) {
	static int sel = 0;
	bool res, sobel;

	if (ImGui::CollapsingHeader("Select shader", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::RadioButton("Phong", &sel, 0); ImGui::SameLine();
		ImGui::RadioButton("Blinn", &sel, 1);
		res = ImGui::Checkbox("Toon-shading", useToon);
		sobel = ImGui::Checkbox("Sobel", useSobel);
		if (*useToon) {
			ImGui::SameLine();
			res |= ImGui::Checkbox("Dithering", useDither);
			const unsigned int one = 1;
			res |= ImGui::InputScalar("Nº tones (diffuse)", ImGuiDataType_U32, nColoresD, &one, NULL, "%u");
			res |= ImGui::InputScalar("Nº tones (specular)", ImGuiDataType_U32, nColoresS, &one, NULL, "%u");
		}
	}

	if (sel != *useBlinn) {
		*useBlinn = sel;
		return true;
	}
	return res;
}

bool imgui_renderSceneSelect(int* nScene, bool* useTextures) {
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

bool imgui_renderCameraPos(float* d, float* az, float* el) {
	bool res = false;

	if (ImGui::CollapsingHeader("Camera position", ImGuiTreeNodeFlags_DefaultOpen)) {
		res |= ImGui::SliderFloat("Distance##camara", d, 2.0f, 16.0f, "%.3f", ImGuiSliderFlags_NoRoundToFormat);
		res |= ImGui::SliderAngle("Azimuth##camara", az, 0.0f, 360.0f, "%.1f deg", ImGuiSliderFlags_NoRoundToFormat);
		res |= ImGui::SliderAngle("Elevation##camara", el, -89.999f, 89.999f, "%.1f deg", ImGuiSliderFlags_NoRoundToFormat);
	}

	return res;
}

bool imgui_renderLightVec(float* az, float* el) {
	bool res = false;

	if (ImGui::CollapsingHeader("Global light vector", ImGuiTreeNodeFlags_DefaultOpen)) {
		res |= ImGui::SliderAngle("Azimuth##luzglobal", az, 0.0f, 360.0f, "%.1f deg", ImGuiSliderFlags_NoRoundToFormat);
		res |= ImGui::SliderAngle("Elevation##luzglobal", el, -89.999f, 89.999f, "%.1f deg", ImGuiSliderFlags_NoRoundToFormat);
	}

	return res;
}

void imgui_renderLightColor(vec3* color) {
	if (ImGui::CollapsingHeader("Lighting color", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::ColorPicker3("##lightColor", &color->r, //Esto es una autentica guarrada que puede explotar en cualquier momento
							ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_DisplayRGB);
	}
}

void imgui_renderCoefficients(vec4* coeficientes) {
	if (ImGui::CollapsingHeader("Lighting coefficients", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::SliderFloat("Ambient##coeficientes", &(coeficientes->x), 0.0f, 1.0f, "%.2f");
		ImGui::SliderFloat("Diffuse##coeficientes", &(coeficientes->y), 0.0f, 1.0f, "%.2f");
		ImGui::SliderFloat("Specular##coeficientes", &(coeficientes->z), 0.0f, 1.0f, "%.2f");
		ImGui::SliderFloat("Shininess##coeficientes", &(coeficientes->w), 1.0f, 100.0f, "%.0f");
	}
}

void imgui_renderBorderSettings(vec3* color, float* t) {
	if (ImGui::CollapsingHeader("Border", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::SliderFloat("Thickness##borderColor", t, 0.0f, 1.0f, "%.1f");
		ImGui::ColorPicker3("##borderColor", &color->r, //Esto es una autentica guarrada que puede explotar en cualquier momento
							ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_DisplayRGB);
	}
}

void imgui_renderframe(void) {
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
