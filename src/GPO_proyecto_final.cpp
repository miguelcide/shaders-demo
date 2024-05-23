/************************  GPO_01 ************************************
ATG, 2019
******************************************************************************/

#include <GpO.h>
#include "GPO_imgui_aux.h"
#include "GPO_assimp_aux.h"

// TAMA�O y TITULO INICIAL de la VENTANA
int ANCHO = 800, ALTO = 600;  // Tama�o inicial ventana
const char* prac = "OpenGL (GpO)";   // Nombre de la practica (aparecera en el titulo de la ventana).

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////   RENDER CODE AND DATA
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

GLFWwindow* window;
GLuint prog[2];
struct {
	GLuint FBO;
	GLuint depth;
	GLuint normals;
	GLuint albedo;
	GLuint position;
} gBuffer;
GLuint bayer, blanco;
GLuint quadVAO;
struct escena isla, torre;
struct escena* escenaActual = &isla;

// Dibuja objeto indexado
void dibujar_indexado(objeto obj) {
	glBindVertexArray(obj.VAO);              // Activamos VAO asociado al objeto
	glDrawElements(GL_TRIANGLES,obj.Ni,obj.tipo_indice,(void*)0);  // Dibujar (indexado)
}

bool useTextures = false;
void dibujar_escena() {
	for (unsigned int i = 0; i < escenaActual->nInstancias; i++) {
		const unsigned int j = escenaActual->instIdx[i];
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, useTextures ? escenaActual->mats[j] : blanco);
		dibujar_indexado(escenaActual->objs[j]);
	}
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void dibujar_quad() {
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, bayer);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gBuffer.albedo);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, gBuffer.depth);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, gBuffer.normals);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, gBuffer.position);

	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);

	glBindTexture(GL_TEXTURE_2D, 0);
}

void crear_gBuffer() {
	glGenTextures(1, &gBuffer.albedo);
	glBindTexture(GL_TEXTURE_2D, gBuffer.albedo);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, ANCHO, ALTO, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gBuffer.albedo, 0);

	glGenTextures(1, &gBuffer.normals);
	glBindTexture(GL_TEXTURE_2D, gBuffer.normals);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, ANCHO, ALTO, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gBuffer.normals, 0);

	glGenTextures(1, &gBuffer.position);
	glBindTexture(GL_TEXTURE_2D, gBuffer.position);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, ANCHO, ALTO, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gBuffer.position, 0);

	glGenTextures(1, &gBuffer.depth);
	glBindTexture(GL_TEXTURE_2D, gBuffer.depth);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, ANCHO, ALTO, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, gBuffer.depth, 0);

	glBindTexture(GL_TEXTURE_2D, 0);
}

// Preparaci�n de los datos de los objetos a dibujar, envialarlos a la GPU
// Compilaci�n programas a ejecutar en la tarjeta gr�fica:  vertex shader, fragment shaders
// Opciones generales de render de OpenGL
void init_scene() {
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);

	// Mandar programas a GPU, compilar y crear programas en GPU
	char* vertex = leer_codigo_de_fichero("data/shaders/prog_pasada_1.vs");
	char* fragment = leer_codigo_de_fichero("data/shaders/prog_pasada_1.fs");
	prog[0] = Compile_Link_Shaders(vertex, fragment);
	delete []vertex;
	delete []fragment;

	vertex = leer_codigo_de_fichero("data/shaders/prog_pasada_2.vs");
	fragment = leer_codigo_de_fichero("data/shaders/prog_pasada_2.fs");
	prog[1] = Compile_Link_Shaders(vertex, fragment);
	delete []vertex;
	delete []fragment;

	//Crear quad
	GLuint quadVBO;
	glGenVertexArrays(1, &quadVAO);
	glBindVertexArray(quadVAO);
	glGenBuffers(1, &quadVBO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, 8*sizeof(float), (float[]){1, -1, 1, 1, -1, -1, -1, 1}, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	//Crear el framebuffer para el g-buffer
	glGenFramebuffers(1, &gBuffer.FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer.FBO);
	crear_gBuffer();
	glDrawBuffers(3, (GLenum[]){GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2});
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//Cargar modelos
	isla = cargar_modelo_assimp("data/Island/Island.obj");
	torre = cargar_modelo_assimp("data/Tower/Tower.obj");
	bayer = cargar_textura("data/bayer16.png", GL_TEXTURE0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	//Literalmente un pixel blanco, para ver el modelo "sin textura"
	glGenTextures(1, &blanco);
	glBindTexture(GL_TEXTURE_2D, blanco);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, (GLubyte[]) {255, 255, 255});
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
}

//Camara
vec3 pos_obs = vec3(8.0f,0.0f,0.0f);
vec3 target = vec3(0.0f,0.0f,0.0f);
vec3 up = vec3(0,1,0);
float fov = 35.0f, aspect = 4.0f / 3.0f;

//Iluminacion
bool useBlinn = false;
vec3 luz = vec3(1, 0, 0);
vec3 colorLuz = vec3(1, 1, 1);
vec4 coeficientes = vec4(0.1, 0.6, 0.3, 16);

//Toon + dither
bool useDither = false;
bool useToon = false;
unsigned int nColoresD = 4;
unsigned int nColoresS = 2;

//Bordes
float grosorBorde = 0.2f;
vec3 colorBorde = vec3(1, 1, 1);

// Actualizar escena: cambiar posici�n objetos, nuevos objetros, posici�n c�mara, luces, etc.
void render_scene() {
	///////// Actualizacion matriz MVP  /////////
	mat4 P = perspective(glm::radians(fov), aspect, 0.5f, 32.0f);
	mat4 V = lookAt(pos_obs, target, up);
	mat4 PV = P * V;

	//Pasada 1
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer.FBO);
	glClearColor(0.0f,0.0f,0.0f,1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(prog[0]);
	transfer_mat4("PV", PV);
	transfer_mat4("M", mat4(1.0f)); //Las escenas ya tienen sus matrices de transformación aplicadas
	transfer_int("baseTexture", 0);
	dibujar_escena();

	//Pasada 2
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClearColor(0.0f,0.0f,0.0f,1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(prog[1]);
	transfer_vec3("camera", pos_obs);
	transfer_vec3("luz", luz);
	transfer_vec3("colorLuz", colorLuz);
	transfer_vec4("coeficientes", coeficientes);
	transfer_float("grosorBorde", grosorBorde);
	transfer_vec3("colorBorde", colorBorde);
	transfer_int("blinn", useBlinn);
	transfer_int("toon", useToon);
	transfer_int("bayer", useDither);
	transfer_uint("nColoresD", nColoresD);
	transfer_uint("nColoresS", nColoresS);
	transfer_vec2("resolution", vec2(ANCHO, ALTO));
	transfer_int("bayerT", 0);
	transfer_int("gAlbedo", 1);
	//Dejar comentado hasta que vayamos a usarlo
	//transfer_int("gDepth", 2);
	transfer_int("gNormals", 3);
	transfer_int("gWorldPos", 4);
	dibujar_quad();
}

//////////  FUNCION PARA PROCESAR VALORES DE IMGUI  //////////
void render_imgui(void) {
	static int nScene = 0;
	static struct {
		float d = 8.0f;
		float az = 0.0f;
		float el = 0.0f;
	} camara;
	static struct {
		float az = 0.0f;
		float el = 0.0f;
	} luzGlobal;

	ImGui::Begin("Controls");

	if (imgui_renderSceneSelect(&nScene, &useTextures)) {
		switch (nScene) {
			case 0:
				escenaActual = &isla;
				break;
			case 1:
				escenaActual = &torre;
				break;
		}
	}
	imgui_renderShaderSelect(&useBlinn, &useToon, &useDither, &nColoresD, &nColoresS);
	if (imgui_renderCameraPos(&camara.d, &camara.az, &camara.el))
		pos_obs = camara.d * vec3(cos(camara.az) * cos(camara.el), sin(camara.el), sin(camara.az) * cos(camara.el));
	if (imgui_renderLightVec(&luzGlobal.az, &luzGlobal.el))
		luz = vec3(cos(luzGlobal.az) * cos(luzGlobal.el), sin(luzGlobal.el), sin(luzGlobal.az) * cos(luzGlobal.el));
	imgui_renderLightColor(&colorLuz);
	imgui_renderCoefficients(&coeficientes);
	imgui_renderBorderSettings(&colorBorde, &grosorBorde);

	ImGui::End();
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// PROGRAMA PRINCIPAL
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
	init_GLFW();            // Inicializa lib GLFW
	window = Init_Window(prac);  // Crea ventana usando GLFW, asociada a un contexto OpenGL	X.Y

	init_imgui(window);

	load_Opengl();         // Carga funciones de OpenGL, comprueba versi�n.
	init_scene();          // Prepara escena
	
	glfwSwapInterval(1);
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		imgui_newframe();

		render_scene();

		//Controles de IMGUI
		render_imgui();

		imgui_renderframe();
		glfwSwapBuffers(window);
		show_info();
	}

	limpiar_escena(&isla);
	limpiar_escena(&torre);
	terminate_imgui();
	glfwTerminate();
	exit(EXIT_SUCCESS);
}

//////////  FUNCION PARA MOSTRAR INFO OPCIONAL EN EL TITULO DE VENTANA  //////////
void show_info()
{
	static int fps = 0;
	static double last_tt = 0;
	double elapsed, tt;
	char nombre_ventana[128];   // buffer para modificar titulo de la ventana

	fps++; tt = glfwGetTime();  // Contador de tiempo en segundos 

	elapsed = (tt - last_tt);
	if (elapsed >= 0.5)  // Refrescar cada 0.5 segundo
	{
		sprintf_s(nombre_ventana, 128, "%s: %4.0f FPS @ %d x %d", prac, fps / elapsed, ANCHO, ALTO);
		glfwSetWindowTitle(window, nombre_ventana);
		last_tt = tt; fps = 0;
	}

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////  ASIGNACON FUNCIONES CALLBACK
///////////////////////////////////////////////////////////////////////////////////////////////////////////


// Callback de cambio tama�o de ventana
void ResizeCallback(GLFWwindow* window, int width, int height)
{
	glfwGetFramebufferSize(window, &width, &height); 
	glViewport(0, 0, width, height);
	ALTO = height;	ANCHO = width;

	aspect = (float)ANCHO / ALTO;

	//Recrear el g-buffer
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer.FBO);
	glDeleteTextures(1, &gBuffer.albedo);
	glDeleteTextures(1, &gBuffer.normals);
	glDeleteTextures(1, &gBuffer.position);
	glDeleteRenderbuffers(1, &gBuffer.depth);
	crear_gBuffer();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// Callback de pulsacion de tecla
static void KeyCallback(GLFWwindow* window, int key, int code, int action, int mode)
{
	switch (key) {
	case GLFW_KEY_ESCAPE:
		glfwSetWindowShouldClose(window, true);
		break;
	}
}


void asigna_funciones_callback(GLFWwindow* window)
{
	glfwSetWindowSizeCallback(window, ResizeCallback);
	glfwSetKeyCallback(window, KeyCallback);
}



