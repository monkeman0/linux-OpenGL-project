#include <iostream>
#include <cmath>
#include <map>
#include <unordered_map>
#include <string>
#include <time.h>
#include <vector>
#include <algorithm>
#include <memory>
#include <thread>
#include <queue>
#include <mutex>

#include "resources/headers/FastNoiseLite.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

//matrix math
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

//font loader
#include <ft2build.h>
#include FT_FREETYPE_H

#include "inputHandler.cpp"
Input input;
Debug debug;
int sliderTester1 = 4;
int sliderTester2 = 40;
float sliderTester3 = 0.f;
short threadsInUse = 0;
short maxThreads = std::thread::hardware_concurrency();

short distanceIncriment[] = {
	1, 2, 4, 8, 10, 10, 20, 20, 20, 20, 20, 20, 40
};
glm::vec3 generatePos = glm::vec3(0.0f, 0.0f, 0.0f);

struct vec2Hash {
	std::size_t operator()(const glm::vec2& v) const {
		std::size_t h1 = std::hash<float>{}(v.x);
		std::size_t h2 = std::hash<float>{}(v.y);
		return h1 ^ (h2 << 1);
	}
};

std::unordered_map<glm::vec2, float, vec2Hash> storedNoise;

#include "resources/headers/tileHandling.h"
#include "data/naturalTiles.cpp"
#include "resources/headers/classes.h"
#include "resources/headers/cameraClass.h"

double pi = 3.141592653589793;
float deltaTime = 0.0f;
float lastTime = 0.0f;

int window_width, window_height;
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::mat4 projection;

#define held(b) input.buttons[b].is_down
#define pressed(b) (input.buttons[b].is_down && input.buttons[b].is_changed)
#define released(b) (!input.buttons[b].is_down && input.buttons[b].is_changed)
#define debugSpeeds()\
if (debug.moveMode == '1') {\
	debug.speed = 0.3f;\
}\
else if(debug.moveMode == '2'){\
	debug.speed = 60.0f;\
}else{\
	debug.speed = 8.0f;\
}\
	camera.MovementSpeed = debug.speed;\

#define debugInputs()\
if (pressed(BUTTON_0))	debug.showWireFrame = !debug.showWireFrame;\
if (pressed(BUTTON_9))  debug.showChunkBorders = !debug.showChunkBorders;\
if (pressed(BUTTON_2)){\
	if(debug.moveMode == '1'){\
	debug.moveMode = '0';\
	}else{\
	debug.moveMode = '1';\
	}\
	debugSpeeds();\
}\
if (pressed(BUTTON_3)){\
	if(debug.moveMode == '2'){\
	debug.moveMode = '0';\
	}else{\
	debug.moveMode = '2';\
	}\
	debugSpeeds();\
}\
if (pressed(BUTTON_ESCAPE)){\
	debug.mouseLocked = !debug.mouseLocked;\
	if (debug.mouseLocked) {\
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);\
		glfwSetCursorPos(window, lastX, lastY);\
		mouse_callback(window, lastX, lastY);\
	}\
	else {\
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);\
		glfwSetCursorPos(window, window_width / 2.f, window_height / 2.f);\
	}\
}\
if (debug.nextFPScounter > debug.FPS) {\
	debug.nextFPScounter = 0;\
	debug.FPS = static_cast<short>(1.0f / deltaTime);\
}\
else {\
	debug.nextFPScounter++;\
}\
if (debug.showWireFrame) {\
glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);\
}\
else {\
glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);\
}\
if (pressed(BUTTON_F11)) {\
	debug.fullscreen = !debug.fullscreen;\
	if (debug.fullscreen) {\
		glfwSetWindowAttrib(window, GLFW_DECORATED, GL_TRUE);\
		glfwSetWindowMonitor(window, NULL, static_cast<int>(mode->width * 0.025), static_cast<int>(mode->height * 0.04), static_cast<int>(mode->width * 0.95), static_cast<int>(mode->height * 0.95), GLFW_DONT_CARE);\
	}\
	else {\
		glfwSetWindowAttrib(window, GLFW_DECORATED, GL_FALSE);\
		glfwSetWindowMonitor(window, NULL, 0, 0, mode->width, mode->height, GLFW_DONT_CARE);\
	}\
}\
\

#define loadNewChunks(renderDistance, pos){\
};\

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void RenderText(Shader& s, std::string text, float x, float y, float scale, glm::vec3 color);
void timeBenchmark(bool stop);
void chunker();

Camera camera(glm::vec3(-12.0f, -75.0f, 42.0f));
float lastX = 800.0f / 2.0f;
float lastY = 800.0f / 2.0f;
bool firstMouse = true;

struct Character {
	unsigned int TextureID;  // ID handle of the glyph texture
	glm::ivec2   Size;       // Size of glyph
	glm::ivec2   Bearing;    // Offset from baseline to left/top of glyph
	unsigned int Advance;    // Offset to advance to next glyph
};

std::map<char, Character> Characters;
unsigned int textVAO, textVBO;

int main() {
	srand(static_cast<unsigned int>(time(NULL)));
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_DECORATED, GL_FALSE);
	//glfwWindowHint(GLFW_SAMPLES, 4);

	const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	window_width = mode->width;
	window_height = mode->height;
	debug.fullscreen = false;
	short extraThreads = std::thread::hardware_concurrency() - 1;

	GLFWwindow* window = glfwCreateWindow(window_width, window_height, "openGL window", NULL/*glfwGetPrimaryMonitor()*/, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSwapInterval(0);

	//call backs
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);


	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330 core");

	std::thread worker1(chunker);
	if (extraThreads < 1)	worker1.join();

	//initialize fonts and openGL settings
	//glEnable(GL_MULTISAMPLE);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
	glEnable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_STENCIL_TEST);
	glStencilMask(0xFF);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	Shader text1Shader("./resources/shaders/text.vert", "./resources/shaders/text.frag");
	glm::mat4 projectionText = glm::ortho(0.0f, float(mode->width), 0.0f, float(mode->height));
	text1Shader.use();
	glUniformMatrix4fv(glGetUniformLocation(text1Shader.ID, "projectionText"), 1, GL_FALSE, glm::value_ptr(projectionText));

	FT_Library ft;
	if (FT_Init_FreeType(&ft))
	{
		std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
		return -1;
	}

	FT_Face face1;
	if (FT_New_Face(ft, "./resources/fonts/mainFont.ttf", 0, &face1))
	{
		std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
		return -1;
	}
	FT_Set_Pixel_Sizes(face1, 0, 48);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // disable byte-alignment restriction

	for (unsigned char c = 0; c < 128; c++)
	{
		// load character glyph 
		if (FT_Load_Char(face1, c, FT_LOAD_RENDER))
		{
			std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
			continue;
		}
		// generate texture
		unsigned int texts;
		glGenTextures(1, &texts);
		glBindTexture(GL_TEXTURE_2D, texts);
		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_RED,
			face1->glyph->bitmap.width,
			face1->glyph->bitmap.rows,
			0,
			GL_RED,
			GL_UNSIGNED_BYTE,
			face1->glyph->bitmap.buffer
		);
		// set texture options
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		// now store character for later use
		Character character = {
			texts,
			glm::ivec2(face1->glyph->bitmap.width, face1->glyph->bitmap.rows),
			glm::ivec2(face1->glyph->bitmap_left, face1->glyph->bitmap_top),
			static_cast<unsigned int>(face1->glyph->advance.x)
		};
		Characters.insert(std::pair<char, Character>(c, character));
	}

	FT_Done_Face(face1);
	FT_Done_FreeType(ft);

	glGenVertexArrays(1, &textVAO);
	glGenBuffers(1, &textVBO);
	glBindVertexArray(textVAO);
	glBindBuffer(GL_ARRAY_BUFFER, textVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	loadTiles();
	//DisplayData(1, 0);
	//shader programs
	Shader shaderProgramBlocks("./resources/shaders/tiles.vert", "./resources/shaders/tiles.frag");
	Shader skyboxShader("./resources/shaders/skybox.vert", "./resources/shaders/skybox.frag");
	Shader lightObjectShader("./resources/shaders/lights.vert", "./resources/shaders/lights.frag");
	Shader lightOutlineShader("./resources/shaders/lights.vert", "./resources/shaders/outline.frag");
	Shader foliageShader("./resources/shaders/foliage.vert", "./resources/shaders/tiles.frag");
	Shader debugShader("./resources/shaders/debug.vert", "./resources/shaders/debug.frag");
	Shader godRaysShader("./resources/shaders/rays.vert", "./resources/shaders/rays.frag");

	//vertex buffer objects and vertex array objects 
	unsigned int lightVAOs[1], lightVBOs[1];
	glGenVertexArrays(1, lightVAOs);
	glBindVertexArray(lightVAOs[0]);
	glGenBuffers(1, &lightVBOs[0]);
	glBindBuffer(GL_ARRAY_BUFFER, lightVBOs[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(halfBlock), halfBlock, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	unsigned int skyboxVAO, skyboxVBO, skyboxEBO;
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glGenBuffers(1, &skyboxEBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skyboxEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(skyboxIndices), &skyboxIndices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	std::string facesCubemap[6] = {
		"./resources/textures/skybox/right.png",
		"./resources/textures/skybox/left.png",
		"./resources/textures/skybox/up.png",
		"./resources/textures/skybox/down.png",
		"./resources/textures/skybox/front.png",
		"./resources/textures/skybox/back.png",
	};

	unsigned int cubemapTexture;
	glGenTextures(1, &cubemapTexture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	for (unsigned int i = 0; i < 6; i++) {
		int width, height, nrChannels;
		unsigned char* data = stbi_load(facesCubemap[i].c_str(), &width, &height, &nrChannels, 0);
		if (data) {
			stbi_set_flip_vertically_on_load(false);
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}
		else {
			std::cout << "Failed to load cubemap texture " << facesCubemap[i] << '\n';
			stbi_image_free(data);
		}
	}



	//use if using indices
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOs[0]);
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indicesSameTextureBlock), indicesSameTextureBlock, GL_DYNAMIC_DRAW);

	Chunk* chunk;
	chunk = new Chunk(0.0f, 0.0f, 0.0f);

	unsigned int meshesAmount = 1;
	Mesh* meshes;
	meshes = new Mesh[meshesAmount];

	/*
	glBindVertexArray(VAOs[0]);
	glBindBuffer(GL_ARRAY_BUFFER, VBOs[0]);
	glBufferData(GL_ARRAY_BUFFER, mainMesh.size(0), mainMesh.vertices, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOs[0]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, mainMesh.size(1), mainMesh.indices, GL_DYNAMIC_DRAW);
	glVertexAttribIPointer(0, 1, GL_UNSIGNED_INT, sizeof(unsigned int), (void*)0);
	glEnableVertexAttribArray(0);


	glBindVertexArray(VAOs[0]);
	glBindBuffer(GL_ARRAY_BUFFER, VBOs[0]);
	glBufferData(GL_ARRAY_BUFFER, mainMesh.size(0), mainMesh.vertices, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOs[0]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, mainMesh.size(1), mainMesh.indices, GL_DYNAMIC_DRAW);
	glVertexAttribIPointer(0, 1, GL_UNSIGNED_INT, 1 * sizeof(unsigned int), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribIPointer(1, 1, GL_UNSIGNED_INT, 3 * sizeof(unsigned int), (void*)(1 * sizeof(unsigned int)));
	glEnableVertexAttribArray(1);
	glVertexAttribIPointer(2, 1, GL_UNSIGNED_INT, 3 * sizeof(unsigned int), (void*)(2 * sizeof(unsigned int)));
	glEnableVertexAttribArray(2);
	*/

	unsigned int foliageVAO, foliageVBO;
	glGenVertexArrays(1, &foliageVAO);
	glGenBuffers(1, &foliageVBO);
	glBindVertexArray(foliageVAO);
	glBindBuffer(GL_ARRAY_BUFFER, foliageVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(foliageVertices), foliageVertices, GL_STATIC_DRAW);
	glVertexAttribIPointer(0, 1, GL_UNSIGNED_INT, 1 * sizeof(unsigned int), (void*)0);
	glEnableVertexAttribArray(0);

	unsigned int debugVAO, debugVBO;
	glGenVertexArrays(1, &debugVAO);
	glGenBuffers(1, &debugVBO);
	glBindVertexArray(debugVAO);
	glBindBuffer(GL_ARRAY_BUFFER, debugVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(debugVertices), &debugVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	unsigned int screenQuadVAO, screenQuadVBO;
	glGenVertexArrays(1, &screenQuadVAO);
	glGenBuffers(1, &screenQuadVBO);
	glBindVertexArray(screenQuadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, screenQuadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(fullScreenQuadVertices), fullScreenQuadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

	unsigned int framebuffer;
	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	unsigned int textureColorbuffer;
	glGenTextures(1, &textureColorbuffer);
	glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, window_width, window_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);

	unsigned int rbo;
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, window_width, window_height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);


	//textures
	stbi_set_flip_vertically_on_load(true);
	const char* atlasLevels[] = {
		"./resources/textures/atlas.png",
		"./resources/textures/atlas(1).png",
		"./resources/textures/atlas(2).png",
		"./resources/textures/atlas(3).png",
		"./resources/textures/atlas(4).png",
		"./resources/textures/atlas(5).png",
	};

	const char* atlasSpecularLevels[] = {
		"./resources/textures/atlasSpecular.png",
		"./resources/textures/atlas(1).png",
		"./resources/textures/atlas(2).png",
		"./resources/textures/atlas(3).png",
		"./resources/textures/atlas(4).png",
		"./resources/textures/atlas(5).png"
	};

	Texture atlas1(atlasLevels, 1, GL_RGBA, 1, 6);
	Texture atlas1Specular(atlasSpecularLevels, 2, GL_RGBA, 1, 1);

	glm::mat4 trans = glm::mat4(1.0f);
	trans = glm::rotate(trans, glm::radians(45.0f), glm::vec3(0.0, 0.0, 1.0));
	trans = glm::scale(trans, glm::vec3(0.5, 0.5, 0.5));

	std::vector<glm::vec3> grassPositions;
	std::vector<float> grassRotations;
	std::vector<int> grassType;

	glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
	glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 cameraDirection = glm::normalize(cameraPos - cameraTarget);
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 cameraRight = glm::normalize(glm::cross(up, cameraDirection));
	glm::vec3 cameraUp = glm::cross(cameraDirection, cameraRight);
	projection = glm::perspective(glm::radians(camera.Zoom), float(window_width) / float(window_height), 0.1f, 1000.0f); // last float is frustum distance

	shaderProgramBlocks.use();
	shaderProgramBlocks.setInt("material.diffuse", 1);
	shaderProgramBlocks.setInt("material.specular", 2);
	debugShader.use();
	debugShader.setInt("aTexture", 1);
	foliageShader.use();
	foliageShader.setInt("material.diffuse", 1);
	foliageShader.setInt("material.specular", 2);

	std::vector<glm::vec3> pointLightPositions = {
		glm::vec3(0.f,  4.f,  0.f),
		glm::vec3(-10.0f, 12.0f, -10.0f),
	};

	std::vector<glm::vec3> glassPositions = {
	   glm::vec3(0.0f,  9.0f,  5.0f),
	   glm::vec3(0.0f, 7.5f, -3.0f),
	   glm::vec3(1.0f, 8.0f, 1.0f),
	};

	float change = 4.1f;
	bool generate = false;
	int completedChunks = 0;
	int genX, genY, genZ = 0;

	while (!glfwWindowShouldClose(window)) {
		//get input changed bools ready to recieve input
		for (int i = 0; i < BUTTON_COUNT; i++) {
			input.buttons[i].is_changed = false;
		}
		//get delta time
		deltaTime = float(glfwGetTime()) - lastTime;
		lastTime = float(glfwGetTime());
		glfwPollEvents();

		glm::vec3 prevPos = glm::vec3(round(camera.Position.x / 10) * 10.0f, round(camera.Position.y / 10) * 10.0f, round(camera.Position.z / 10) * 10.0f);
		//std::cout << "X: " << prevPos.x << " Y: " << prevPos.y << " Z: " << prevPos.z << '\n';

		if (held(BUTTON_W))  camera.ProcessKeyboard(FORWARD, deltaTime);
		if (held(BUTTON_S))  camera.ProcessKeyboard(BACKWARD, deltaTime);
		if (held(BUTTON_A))  camera.ProcessKeyboard(LEFT, deltaTime);
		if (held(BUTTON_D))	 camera.ProcessKeyboard(RIGHT, deltaTime);
		if (held(BUTTON_SPACE))	camera.ProcessKeyboard(UP, deltaTime);
		if (held(BUTTON_LEFTSHIFT)) camera.ProcessKeyboard(DOWN, deltaTime);
		camera.Roll *= (abs((atan(deltaTime * 10.0f) - 95.f) * 0.01f) * (abs(camera.Roll) > 0.05f));
		camera.updateCameraVectors();

		glm::vec3 newPos = glm::vec3(round(camera.Position.x / 10) * 10.0f, round(camera.Position.y / 10) * 10.0f, round(camera.Position.z / 10) * 10.0f);

		if (newPos != prevPos) {

		} //loadNewChunks(sliderTester1, newPos);
		glm::mat4 model = glm::mat4(1.0f);
		glm::mat4 view = camera.GetViewMatrix();

		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

		glEnable(GL_DEPTH_TEST);
		//glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		glStencilMask(0xFF);
		glStencilFunc(GL_ALWAYS, 0, 0xFF);

		shaderProgramBlocks.use();
		shaderProgramBlocks.setMat4("pv", projection * view);
		shaderProgramBlocks.setVec3("viewPos", camera.Position);

		shaderProgramBlocks.setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
		shaderProgramBlocks.setVec3("dirLight.ambient", 0.02f, 0.02f, 0.02f);
		shaderProgramBlocks.setVec3("dirLight.diffuse", 1.2f, 1.15f, 1.15f);
		shaderProgramBlocks.setVec3("dirLight.specular", 0.7f, 0.65f, 0.65f);

		shaderProgramBlocks.setVec3("pointLights[0].position", pointLightPositions[0]);
		shaderProgramBlocks.setVec3("pointLights[0].ambient", 0.05f, 0.05f, 0.05f);
		shaderProgramBlocks.setVec3("pointLights[0].diffuse", 8.0f, 8.0f, 8.0f);
		shaderProgramBlocks.setVec3("pointLights[0].specular", 4.0f, 4.0f, 4.0f);
		shaderProgramBlocks.setFloat("pointLights[0].constant", 1.0f);
		shaderProgramBlocks.setFloat("pointLights[0].linear", 0.09f);
		shaderProgramBlocks.setFloat("pointLights[0].quadratic", 0.032f);

		shaderProgramBlocks.setVec3("pointLights[1].position", pointLightPositions[1]);
		shaderProgramBlocks.setVec3("pointLights[1].ambient", 0.05f, 0.05f, 0.05f);
		shaderProgramBlocks.setVec3("pointLights[1].diffuse", 0.8f, 0.8f, change);
		shaderProgramBlocks.setVec3("pointLights[1].specular", 1.0f, 1.0f, 1.0f);
		shaderProgramBlocks.setFloat("pointLights[1].constant", 1.0f);
		shaderProgramBlocks.setFloat("pointLights[1].linear", 0.09f);
		shaderProgramBlocks.setFloat("pointLights[1].quadratic", 0.032f);

		shaderProgramBlocks.setVec3("spotLight.position", camera.Position);
		shaderProgramBlocks.setVec3("spotLight.direction", camera.Front);
		shaderProgramBlocks.setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
		shaderProgramBlocks.setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
		shaderProgramBlocks.setVec3("spotLight.specular", 3.0f, 3.0f, 3.0f);
		shaderProgramBlocks.setFloat("spotLight.constant", 0.2f);
		shaderProgramBlocks.setFloat("spotLight.linear", 0.09f);
		shaderProgramBlocks.setFloat("spotLight.quadratic", 0.032f);
		shaderProgramBlocks.setFloat("spotLight.cutOff", glm::cos(glm::radians(15.0f)));
		shaderProgramBlocks.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(17.5f)));

		if (generate == false) {
			for (unsigned int i = 0; i < completedChunks; i++) {
				if (meshes[i].length > 0) {
					glBindVertexArray(meshes[i].VAO);
					model = glm::mat4(1.0f);
					model = glm::translate(model, glm::vec3(meshes[i].X, meshes[i].Y, meshes[i].Z));
					shaderProgramBlocks.setFloat("LODstep", float(meshes[i].distanceI));
					shaderProgramBlocks.setMat4("model", model);
					shaderProgramBlocks.setFloat("material.shininess", 32.f);
					//glDrawElements(GL_TRIANGLES, meshes[i].indicesAmount, GL_UNSIGNED_INT, 0);
					glDrawArrays(GL_TRIANGLES, 0, meshes[i].length);
				}
			}
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDisable(GL_DEPTH_TEST);
		glClear(GL_COLOR_BUFFER_BIT);

		skyboxShader.use();
		skyboxShader.setInt("skybox", 0);
		view = glm::mat4(glm::mat3(camera.GetViewMatrix()));
		skyboxShader.setMat4("pv", projection * view);
		glBindVertexArray(skyboxVAO);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

		godRaysShader.use();
		godRaysShader.setBool("rays", false);
		glBindVertexArray(screenQuadVAO);
		glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		godRaysShader.setBool("rays", true);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		//glDrawElements(GL_TRIANGLES, mainMesh.indicesAmount, GL_UNSIGNED_INT, 0);

		/*foliageShader.use();
		foliageShader.setMat4("pv", projection * view);
		foliageShader.setVec3("viewPos", camera.Position);
		foliageShader.setFloat("utime", float(glfwGetTime()));

		foliageShader.setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
		foliageShader.setVec3("dirLight.ambient", 0.02f, 0.02f, 0.02f);
		foliageShader.setVec3("dirLight.diffuse", 0.1f, 0.1f, 0.1f);
		foliageShader.setVec3("dirLight.specular", 0.15f, 0.15f, 0.15f);

		foliageShader.setVec3("pointLights[0].position", pointLightPositions[0]);
		foliageShader.setVec3("pointLights[0].ambient", 0.05f, 0.05f, 0.05f);
		foliageShader.setVec3("pointLights[0].diffuse", 8.0f, 8.0f, 8.0f);
		foliageShader.setVec3("pointLights[0].specular", 4.0f, 4.0f, 4.0f);
		foliageShader.setFloat("pointLights[0].constant", 1.0f);
		foliageShader.setFloat("pointLights[0].linear", 0.09f);
		foliageShader.setFloat("pointLights[0].quadratic", 0.032f);

		foliageShader.setVec3("pointLights[1].position", pointLightPositions[1]);
		foliageShader.setVec3("pointLights[1].ambient", 0.05f, 0.05f, 0.05f);
		foliageShader.setVec3("pointLights[1].diffuse", 0.8f, 0.8f, change);
		foliageShader.setVec3("pointLights[1].specular", 1.0f, 1.0f, 1.0f);
		foliageShader.setFloat("pointLights[1].constant", 1.0f);
		foliageShader.setFloat("pointLights[1].linear", 0.09f);
		foliageShader.setFloat("pointLights[1].quadratic", 0.032f);

		foliageShader.setVec3("spotLight.position", camera.Position);
		foliageShader.setVec3("spotLight.direction", camera.Front);
		foliageShader.setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
		foliageShader.setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
		foliageShader.setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
		foliageShader.setFloat("spotLight.constant", 1.0f);
		foliageShader.setFloat("spotLight.linear", 0.09f);
		foliageShader.setFloat("spotLight.quadratic", 0.032f);
		foliageShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
		foliageShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));
		glBindVertexArray(VAOs[2]);

		for (int i = 0; i < grassPositions.size(); i++) {
			model = glm::mat4(1.0f);
			model = glm::translate(model, grassPositions[i]);
			model = glm::rotate(model, glm::radians(grassRotations[i]), glm::vec3(0.0f, 1.0f, 0.0f));
			foliageShader.setMat4("model", model);
			foliageShader.setFloat("material.shininess", 64.f);
			if (grassType[i]) {
				glDrawArrays(GL_TRIANGLE_STRIP, 132, 96);
			}
			else {
				glDrawArrays(GL_TRIANGLE_STRIP, 0, 72);
				glDrawArrays(GL_TRIANGLE_STRIP, 72, 60);
			}
		}

		glStencilMask(0xFF);
		glStencilFunc(GL_ALWAYS, 1, 0xFF);

		lightObjectShader.use();
		lightObjectShader.setMat4("projection", projection);
		lightObjectShader.setMat4("view", view);
		glBindVertexArray(lightVAOs[0]);

		for (unsigned int j = 0; j < 2; j++)
		{
			model = glm::mat4(1.0f);
			model = glm::translate(model, pointLightPositions[j]);
			model = glm::scale(model, glm::vec3(0.4f, 0.4f, 0.4f));
			lightObjectShader.setMat4("model", model);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

		glStencilMask(0xFF);
		glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
		//glDisable(GL_DEPTH_TEST);

		lightOutlineShader.use();
		lightOutlineShader.setMat4("projection", projection);
		lightOutlineShader.setMat4("view", view);

		std::map<float, glm::vec3> transPos;
		for (unsigned int i = 0; i < pointLightPositions.size(); i++)
		{
			float distance = glm::length(camera.Position - pointLightPositions[i]);
			transPos[distance] = pointLightPositions[i];
		}

		for (std::map<float, glm::vec3>::reverse_iterator it = transPos.rbegin(); it != transPos.rend(); ++it) {
			model = glm::mat4(1.0f);
			model = glm::translate(model, it->second);
			model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
			lightOutlineShader.setMat4("model", model);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

		shaderProgramBlocks.use();
		glBindVertexArray(VAOs[0]);

		glStencilMask(0xFF);
		glStencilFunc(GL_ALWAYS, 1, 0xFF);

		std::map<float, glm::vec3> transPos2;
		for (unsigned int i = 0; i < glassPositions.size(); i++)
		{
			float distance = glm::length(camera.Position - glassPositions[i]);
			transPos2[distance] = glassPositions[i];
		}

		for (std::map<float, glm::vec3>::reverse_iterator it = transPos2.rbegin(); it != transPos2.rend(); ++it) {
			model = glm::mat4(1.0f);
			model = glm::translate(model, it->second);
			model = glm::scale(model, glm::vec3(2.0f, 2.0f, 2.0f));
			shaderProgramBlocks.setMat4("model", model);
			glDrawArrays(GL_TRIANGLES, 72, 6);
		}*/
		if (pressed(BUTTON_7)) {
			for (unsigned int i = 0; i < meshesAmount; i++) {
				std::cout << sizeof(meshes[i]) + meshes[i].length * 4 << " bytes, " << (sizeof(meshes[i]) + meshes[i].length * 4) / 1000 << " kilobytes\n";
			}
		}
		if (pressed(BUTTON_8) && generate == false) {
			timeBenchmark(0);
			unsigned int count = 0;
			meshesAmount = 0;
			generatePos = newPos;
			for (int y = -1 - (sliderTester1 * 0.15); y < 1 + (sliderTester1 * 0.3); y++) {
				for (int z = sliderTester1 * -1; z < sliderTester1; z++) {
					for (int x = sliderTester1 * -1; x < sliderTester1; x++) {
						if (sqrt((x * x) + (y * y) * 4 + (z * z)) <= sliderTester1) {
							meshesAmount++;
						}
					}
				}
			}

			delete[] meshes;
			meshes = new Mesh[meshesAmount];
			generate = true;
			completedChunks = 0;
			genY = -1 - (sliderTester1 * 0.15);
			genZ = sliderTester1 * -1;
			genX = sliderTester1 * -1;
			//loadNewChunks(sliderTester1, newPos);
		}
		if (generate) {
			if (sqrt((genX * genX) + (genY * genY) * 4 + (genZ * genZ)) <= sliderTester1) {
				chunk->create(generatePos.x + float(genX * 10), generatePos.y + float(genY * 10), generatePos.z + float(genZ * 10));
				meshes[completedChunks].fillChunk(*chunk);
				//pthread_create(&worker1, NULL, chunker, glm::vec3(generatePos.x + float(genX * 10), generatePos.y + float(genY * 10), generatePos.z + float(genZ * 10)), );
				static int lastPercent = -1;
				int percent = int((float(completedChunks) / meshesAmount) * 100);
				if (percent != lastPercent) {
					std::cout << percent << "%\n";
					lastPercent = percent;
				}
				completedChunks++;
			}
			if (genX < sliderTester1 - 1) {
				genX++;
			}
			else if (genZ < sliderTester1 - 1) {
				genX = sliderTester1 * -1;
				genZ++;
			}
			else if (genY < (sliderTester1 * 0.3)) {
				genZ = sliderTester1 * -1;
				genY++;
			}
			else {
				generate = false;
				timeBenchmark(1);
			}

		}



		/*if (held(BUTTON_9)) {
			chunk->create(sliderTester1 * chunk->voxelWidths, sliderTester2 * chunk->voxelWidths, sliderTester3 * chunk->voxelWidths);
			mainMesh.fillChunk(*chunk);
			mainMesh.updateBuffers();
		}*/
		if (debug.showChunkBorders) {
			glDisable(GL_CULL_FACE);
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);\
				debugShader.use();
			debugShader.setMat4("pv", projection * view);
			glBindVertexArray(debugVAO);

			for (unsigned int i = 0; i < meshesAmount; i++) {
				//if (sqrt((meshes[i].X - camera.Position.x) * (meshes[i].X - camera.Position.x) + (meshes[i].Y - camera.Position.y) * (meshes[i].Y - camera.Position.y) + (meshes[i].Z - camera.Position.z) * (meshes[i].Z - camera.Position.z)) < 40) {
				model = glm::mat4(1.0f);
				model = glm::translate(model, glm::vec3(meshes[i].X, meshes[i].Y, meshes[i].Z));
				if (meshes[i].length == 0) {
					debugShader.setBool("skipped", true);
				}
				else {
					debugShader.setBool("skipped", false);
				}
				debugShader.setMat4("model", model);
				glDrawArrays(GL_TRIANGLES, 0, 36);
				//}
			}
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);\
				glEnable(GL_CULL_FACE);
		}

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		debugInputs();
		ImGui::Begin("ImGui window");
		ImGui::SliderFloat("value", &change, -100.0f, 100.0f, "%.3f", 0);
		ImGui::Text("FPS: = %i", debug.FPS);
		ImGui::Text("X: = %f, Y: %f, Z: %f", camera.Position.x, camera.Position.y, camera.Position.z);
		//ImGui::Text("Vertices Amount: = %i", (mainMesh.length / 3));
		//ImGui::Text("Indices Amount: = %i", (mainMesh.indicesAmount));
		ImGui::SliderInt("Render Distance", &sliderTester1, 1, 50, "%.1i", 0);
		ImGui::SliderFloat("Test", &sliderTester3, -1.5f, 1.5f, "%.3f", 0);
		ImGui::End();
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
	}
	worker1.join();
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	delete chunk;
	glDeleteVertexArrays(1, &foliageVAO);
	glDeleteBuffers(1, &foliageVBO);
	shaderProgramBlocks.Delete();
	foliageShader.Delete();
	text1Shader.Delete();
	glfwTerminate();
	return 0;
}


void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	window_width = width;
	window_height = height;
	projection = glm::perspective(glm::radians(camera.Zoom), float(window_width) / float(window_height), 0.1f, 1000.0f);
	glViewport(0, 0, width, height);
}

void RenderText(Shader& s, std::string text, float x, float y, float scale, glm::vec3 color) {

	// activate corresponding render state	
	s.use();
	glUniform3f(glGetUniformLocation(s.ID, "textColor"), color.x, color.y, color.z);
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(textVAO);

	// iterate through all characters
	std::string::const_iterator c;
	for (c = text.begin(); c != text.end(); c++)
	{
		Character ch = Characters[*c];

		float xpos = x + ch.Bearing.x * scale;
		float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

		float w = ch.Size.x * scale;
		float h = ch.Size.y * scale;
		// update VBO for each character
		float vertices[6][4] = {
			{ xpos,     ypos + h,   0.0f, 0.0f },
			{ xpos,     ypos,       0.0f, 1.0f },
			{ xpos + w, ypos,       1.0f, 1.0f },

			{ xpos,     ypos + h,   0.0f, 0.0f },
			{ xpos + w, ypos,       1.0f, 1.0f },
			{ xpos + w, ypos + h,   1.0f, 0.0f }
		};
		// render glyph texture over quad
		glBindTexture(GL_TEXTURE_2D, ch.TextureID);
		// update content of VBO memory
		glBindBuffer(GL_ARRAY_BUFFER, textVBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		// render quad
		glDrawArrays(GL_TRIANGLES, 0, 6);
		// now advance cursors for next glyph (note that advance is number of 1/64 pixels)
		x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64)
	}
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (action == 2) {
		action = 1;
	}

#define process_button(b, keyCode)\
case keyCode: {\
input.buttons[b].is_changed = (bool(action) != input.buttons[b].is_down);\
input.buttons[b].is_down = bool(action);\
} break;\

	switch (key) {
		process_button(BUTTON_SPACE, 32);
		process_button(BUTTON_F3, 292);
		process_button(BUTTON_0, 48);
		process_button(BUTTON_1, 49);
		process_button(BUTTON_2, 50);
		process_button(BUTTON_3, 51);
		process_button(BUTTON_4, 52);
		process_button(BUTTON_5, 53);
		process_button(BUTTON_6, 54);
		process_button(BUTTON_7, 55);
		process_button(BUTTON_8, 56);
		process_button(BUTTON_9, 57);
		process_button(BUTTON_F11, 300);
		process_button(BUTTON_W, 87);
		process_button(BUTTON_A, 65);
		process_button(BUTTON_S, 83);
		process_button(BUTTON_D, 68);
		process_button(BUTTON_LEFTSHIFT, 340);
		process_button(BUTTON_ESCAPE, 256);
		process_button(BUTTON_F4, 293);
	default:
		break;
	}
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
	if (debug.mouseLocked) {
		float xpos = static_cast<float>(xposIn);
		float ypos = static_cast<float>(yposIn);

		if (firstMouse)
		{
			lastX = xpos;
			lastY = ypos;
			firstMouse = false;
		}

		float xoffset = xpos - lastX;
		float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

		lastX = xpos;
		lastY = ypos;
		camera.ProcessMouseMovement(xoffset, yoffset);
	}
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	if (debug.mouseLocked) {
		camera.ProcessMouseScroll(static_cast<float>(yoffset));
		projection = glm::perspective(glm::radians(camera.Zoom), float(window_width) / float(window_height), 0.1f, 1000.0f);
	}
}

void timeBenchmark(bool stop) {
	static float oldTime;
	if (stop) {
		std::cout << "Total time: " << float(glfwGetTime()) - oldTime << '\n';
	}
	else {
		oldTime = float(glfwGetTime());
		std::cout << "Starting benchmark\n";
	}
}

void chunker() {
	while (true) {
		//std::cout << "working\n";
	}
}