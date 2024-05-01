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

bool imgui_renderShaderSelect(int* nProg) {
	static int sel = 0;

	ImGui::Text("Select shader");
	ImGui::RadioButton("Phong", &sel, 0); ImGui::SameLine();
	ImGui::RadioButton("Blinn", &sel, 1);

	if (sel != *nProg) {
		*nProg = sel;
		return true;
	}
	return false;
}

bool imgui_renderCameraPos(float* d, float* az, float* el) {
	bool res = false;

	ImGui::Text("Camera position");
	res |= ImGui::SliderFloat("Distance##camara", d, 2.0f, 16.0f, "%.3f", ImGuiSliderFlags_NoRoundToFormat);
	res |= ImGui::SliderAngle("Azimuth##camara", az, 0.0f, 360.0f, "%.1f deg", ImGuiSliderFlags_NoRoundToFormat);
	res |= ImGui::SliderAngle("Elevation##camara", el, -89.999f, 89.999f, "%.1f deg", ImGuiSliderFlags_NoRoundToFormat);

	return res;
}

bool imgui_renderLightVec(float* az, float* el) {
	bool res = false;

	ImGui::Text("Global light vector");
	res |= ImGui::SliderAngle("Azimuth##luzglobal", az, 0.0f, 360.0f, "%.1f deg", ImGuiSliderFlags_NoRoundToFormat);
	res |= ImGui::SliderAngle("Elevation##luzglobal", el, -89.999f, 89.999f, "%.1f deg", ImGuiSliderFlags_NoRoundToFormat);

	return res;
}

void imgui_renderLightColor(vec3* color) {
	ImGui::Text("Lighting color");
	ImGui::ColorPicker3("##color", &color->r, //Esto es una autentica guarrada que puede explotar en cualquier momento
		ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_DisplayRGB);
}

void imgui_renderCoefficients(vec4* coeficientes) {
	ImGui::Text("Lighting coefficients");
	ImGui::SliderFloat("Ambient##coeficientes", &(coeficientes->x), 0.0f, 1.0f, "%.2f");
	ImGui::SliderFloat("Diffuse##coeficientes", &(coeficientes->y), 0.0f, 1.0f, "%.2f");
	ImGui::SliderFloat("Specular##coeficientes", &(coeficientes->z), 0.0f, 1.0f, "%.2f");
	ImGui::SliderFloat("Shininess##coeficientes", &(coeficientes->w), 1.0f, 100.0f, "%.0f");
}

void imgui_renderframe(void) {
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
