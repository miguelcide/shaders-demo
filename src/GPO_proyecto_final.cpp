/************************  GPO_01 ************************************
ATG, 2019
******************************************************************************/

#include <GpO.h>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

// TAMA�O y TITULO INICIAL de la VENTANA
int ANCHO = 800, ALTO = 600;  // Tama�o inicial ventana
const char* prac = "OpenGL (GpO)";   // Nombre de la practica (aparecera en el titulo de la ventana).

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////   RENDER CODE AND DATA
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

GLFWwindow* window;
GLuint prog;
objeto obj;

// Dibuja objeto indexado
void dibujar_indexado(objeto obj) {
	glBindVertexArray(obj.VAO);              // Activamos VAO asociado al objeto
	glDrawElements(GL_TRIANGLES,obj.Ni,obj.tipo_indice,(void*)0);  // Dibujar (indexado)
	glBindVertexArray(0);
}

mat4 PV;
vec3 pos_obs = vec3(0.0f,0.0f,5.0f);
vec3 target = vec3(0.0f,0.0f,0.0f);
vec3 up = vec3(0,1,0);
float fov = 35.0f, aspect = 4.0f / 3.0f;

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
	char* fragment = leer_codigo_de_fichero("data/prog.fs");
	prog = Compile_Link_Shaders(vertex, fragment);
	delete []vertex;
	delete []fragment;

	glUseProgram(prog);

	mat4 P = perspective(glm::radians(fov), aspect, 0.5f, 20.0f);
	mat4 V = lookAt(pos_obs, target, up);
	PV = P * V;

	obj = cargar_modelo("data/buda_n.bix");

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
}

vec3 luz = vec3(1, 1, 0) / sqrt(2.0f);
vec4 coeficientes = vec4(0.1, 0.6, 0.3, 16);

// Actualizar escena: cambiar posici�n objetos, nuevos objetros, posici�n c�mara, luces, etc.
void render_scene()
{
	glClearColor(0.0f,0.0f,0.0f,1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	float t = (float)glfwGetTime();  // Contador de tiempo en segundos 

	///////// Actualizacion matriz MVP  /////////	
	mat4 M = translate(vec3(0.0f, -1.0f, 0.0f)) * rotate(1.5f * t, vec3(0.0f, 1.0f, 0.0f));

	transfer_mat4("PV", PV);
	transfer_mat4("M", M);
	transfer_vec3("camera", pos_obs);
	transfer_vec3("luz", luz);
	transfer_vec4("coeficientes", coeficientes);

	// ORDEN de dibujar
	dibujar_indexado(obj);
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// PROGRAMA PRINCIPAL
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
	init_GLFW();            // Inicializa lib GLFW
	window = Init_Window(prac);  // Crea ventana usando GLFW, asociada a un contexto OpenGL	X.Y

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

	load_Opengl();         // Carga funciones de OpenGL, comprueba versi�n.
	init_scene();          // Prepara escena
	
	glfwSwapInterval(1);
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		render_scene();
		//ImGui::ShowDemoWindow();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glfwSwapBuffers(window);
		show_info();
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
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
	mat4 P = perspective(glm::radians(fov), aspect, 0.5f, 20.0f);
	mat4 V = lookAt(pos_obs, target, up);
	PV = P * V;
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



