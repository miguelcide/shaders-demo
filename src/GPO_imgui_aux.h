#pragma once
#include <GpO.h>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

void init_imgui(GLFWwindow* window);
void terminate_imgui(void);
void imgui_newframe(void);
void imgui_renderframe(void);

void imgui_renderShaderSelect(bool* useBlinn, bool* useToon, bool* useDither, bool* useHatching, bool* useSobelTex,
								bool* useSobelNorm, unsigned int* nColoresD, unsigned int* nColoresS, bool* improved_border);
bool imgui_renderSceneSelect(int* nScene, bool* useTextures);
bool imgui_renderCameraPos(float* d, float* az, float* el);
bool imgui_renderLightVec(float* az, float* el);
void imgui_renderLightColor(vec3* lightColor, vec3* bgColor);
void imgui_renderCoefficients(vec4* coeficientes);
void imgui_renderBorderSettings(vec3* color, float* grosorBorde);
void imgui_renderImprovedBorderSettings(vec3* color, float* tex_threshold, float* normal_threshold);
