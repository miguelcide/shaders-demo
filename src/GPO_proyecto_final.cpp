/************************  GPO_01 ************************************
ATG, 2019
******************************************************************************/

#include <GpO.h>
#include "GPO_imgui_aux.h"

// TAMA�O y TITULO INICIAL de la VENTANA
int ANCHO = 800, ALTO = 600;  // Tama�o inicial ventana
const char* prac = "OpenGL (GpO)";   // Nombre de la practica (aparecera en el titulo de la ventana).

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////   RENDER CODE AND DATA
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

GLFWwindow* window;
GLuint prog[2];
objeto obj;

// Dibuja objeto indexado
void dibujar_indexado(objeto obj) {
	glBindVertexArray(obj.VAO);              // Activamos VAO asociado al objeto
	glDrawElements(GL_TRIANGLES,obj.Ni,obj.tipo_indice,(void*)0);  // Dibujar (indexado)
	glBindVertexArray(0);
}

// Preparaci�n de los datos de los objetos a dibujar, envialarlos a la GPU
// Compilaci�n programas a ejecutar en la tarjeta gr�fica:  vertex shader, fragment shaders
// Opciones generales de render de OpenGL
void init_scene()
{
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);

	// Mandar programas a GPU, compilar y crear programa en GPU
	char* vertex = leer_codigo_de_fichero("data/prog.vs");
	char* fragment = leer_codigo_de_fichero("data/prog_phong.fs");
	prog[0] = Compile_Link_Shaders(vertex, fragment);
	delete []vertex;
	delete []fragment;

	vertex = leer_codigo_de_fichero("data/prog.vs");
	fragment = leer_codigo_de_fichero("data/prog_blinn.fs");
	prog[1] = Compile_Link_Shaders(vertex, fragment);
	delete []vertex;
	delete []fragment;

	glUseProgram(prog[0]);

	obj = cargar_modelo("data/buda_n.bix");

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
}

vec3 pos_obs = vec3(8.0f,0.0f,0.0f);
vec3 target = vec3(0.0f,0.0f,0.0f);
vec3 up = vec3(0,1,0);
float fov = 35.0f, aspect = 4.0f / 3.0f;

vec3 luz = vec3(1, 1, 0) / sqrt(2.0f);
vec4 coeficientes = vec4(0.1, 0.6, 0.3, 16);

// Actualizar escena: cambiar posici�n objetos, nuevos objetros, posici�n c�mara, luces, etc.
void render_scene()
{
	glClearColor(0.0f,0.0f,0.0f,1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	float t = (float)glfwGetTime();  // Contador de tiempo en segundos 

	///////// Actualizacion matriz MVP  /////////
	mat4 P = perspective(glm::radians(fov), aspect, 0.5f, 32.0f);
	mat4 V = lookAt(pos_obs, target, up);
	mat4 PV = P * V;
	mat4 M = translate(vec3(0.0f, -1.0f, 0.0f));

	transfer_mat4("PV", PV);
	transfer_mat4("M", M);
	transfer_vec3("camera", pos_obs);
	transfer_vec3("luz", luz);
	transfer_vec4("coeficientes", coeficientes);

	// ORDEN de dibujar
	dibujar_indexado(obj);
}

//////////  FUNCION PARA PROCESAR VALORES DE IMGUI  //////////
void render_imgui(void) {
	static int nProg = 0;
	static float d = 8.0f;
	static float az = 0.0f;
	static float el = 0.0f;

	ImGui::Begin("Controls");

	if (imgui_renderShaderSelect(&nProg))
		glUseProgram(prog[nProg]);
	if (imgui_renderCameraPos(&d, &az, &el))
		pos_obs = d * vec3(cos(az) * cos(el), sin(el), sin(az) * cos(el));

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



