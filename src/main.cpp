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

#include "glad.h"
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "FastNoiseLite.h"

#include <ft2build.h>
#include FT_FREETYPE_H

#include "inputHandler.cpp"
Input input;
Debug debug;
float sliderTester1 = 35;
float sliderTester2 = 40;
float sliderTester3 = 0.f;
float t1, t2, t3, t4, t5 = 0.f;

short distanceIncriment[] = {
	1, 1, 2, 2, 4, 4, 8, 10, 10, 20, 20, 20, 20, 20, 20, 40
};
glm::vec3 generatePos = glm::vec3(0.0f, 0.0f, 0.0f);

struct vec2Hash {
    std::size_t operator()(const glm::vec2& v) const {
        std::size_t h1 = std::hash<float>{}(v.x);
        std::size_t h2 = std::hash<float>{}(v.y);
        return h1 ^ (h2 << 1);
    }
};

struct vec3Hash {
    std::size_t operator()(const glm::vec3& v) const noexcept {
        std::size_t h = std::hash<float>{}(v.x);
        auto combine = [](std::size_t seed, std::size_t value) {
            return seed ^ (value + 0x9e3779b9 + (seed << 6) + (seed >> 2));
        };
        h = combine(h, std::hash<float>{}(v.y));
        h = combine(h, std::hash<float>{}(v.z));
        return h;
    }
};

std::unordered_map<glm::vec2, float, vec2Hash> storedNoise;
int totalChunksGenerated = 0;
float bytesFromMeshes = 0;

#include "tileHandling.h"
#include "naturalTiles.cpp"
#include "classes.h"
#include "cameraClass.h"

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
if (held(BUTTON_LEFT_CTRL) && pressed(BUTTON_1))	debug.useLOD = !debug.useLOD;\
if (held(BUTTON_LEFT_CTRL) && held(BUTTON_LEFTSHIFT) && pressed(BUTTON_P)) glfwSetWindowShouldClose(window, GLFW_TRUE);\
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

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void RenderText(Shader& s, std::string text, float x, float y, float scale, glm::vec3 color);
void timeBenchmark(bool stop);
void setSpawn(float x, float z);
void chunker1();

Chunk* worker1chunk;
Chunk* chunk;
Mesh* meshes;

int totalChunks = 0;
int neededThreadGeneratedChunks = 0;
bool generate = false;
bool worker1Finished = true;
bool createAllMeshes = false;
unsigned int meshesAmount = 0;
unsigned int completedChunks = 0;
std::vector<unsigned int> realMeshes;

struct chunkCoords {
	float x, y, z;
	unsigned int index;
};

std::vector<chunkCoords> neededChunks;
std::vector<unsigned int> neededMeshBuffers;

bool active = false;
bool joinableThreads[1] = { true };
std::thread worker1(chunker1);

Camera camera(glm::vec3(0, 0, 0));
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
unsigned int rbo;
unsigned int textureColorbuffer;
unsigned int framebuffer;
bool makeChunksOrder = false;

struct Plane {
    glm::vec3 normal;
    float distance;

    Plane() = default;

	Plane(const glm::vec3& p1, const glm::vec3& norm)
		: normal(glm::normalize(norm)),
		distance(glm::dot(normal, p1))
	{}

	float getSignedDistanceToPlane(const glm::vec3& point) const {
    return glm::dot(normal, point) - distance;
	}
};

struct Frustum {
	Plane topFace;
	Plane bottomFace;

	Plane leftFace;
	Plane rightFace;

	Plane nearFace;
	Plane farFace;
};

Frustum createFrustumFromCamera(const Camera& cam, float aspect, float fovY, float zNear, float zFar)
{
    Frustum frustum;

    glm::vec3 nearCenter = cam.Position + cam.Front * zNear;
    glm::vec3 farCenter  = cam.Position + cam.Front * zFar;

    float halfVSide = zFar * tanf(fovY * 0.5f);
    float halfHSide = halfVSide * aspect;

    glm::vec3 up = cam.Up;
    glm::vec3 right = cam.Right;
    glm::vec3 front = cam.Front;

    // Near and Far planes
    frustum.nearFace = Plane(nearCenter, front);
    frustum.farFace  = Plane(farCenter, -front);

    // Top plane
    glm::vec3 topNormal = glm::cross(glm::normalize(glm::cross(right, front)), right);
    glm::vec3 topPoint = cam.Position + up * halfVSide + front * zFar;
    frustum.topFace = Plane(topPoint, glm::cross(glm::cross(right, front), right));

    // Bottom plane
    glm::vec3 bottomNormal = glm::cross(right, glm::normalize(glm::cross(front, -right)));
    glm::vec3 bottomPoint = cam.Position - up * halfVSide + front * zFar;
    frustum.bottomFace = Plane(bottomPoint, glm::cross(right, glm::cross(front, -right)));

    // Right plane
    glm::vec3 rightNormal = glm::cross(up, glm::normalize(glm::cross(front, up)));
    glm::vec3 rightPoint = cam.Position + right * halfHSide + front * zFar;
    frustum.rightFace = Plane(rightPoint, glm::cross(up, glm::cross(front, up)));

    // Left plane
    glm::vec3 leftNormal = glm::cross(glm::normalize(glm::cross(up, front)), up);
    glm::vec3 leftPoint = cam.Position - right * halfHSide + front * zFar;
    frustum.leftFace = Plane(leftPoint, glm::cross(glm::cross(up, front), up));

    return frustum;
}

struct Transform {
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;
	glm::mat4 model;

    Transform(glm::vec3 p, glm::vec3 r, glm::vec3 s, glm::mat4 m)
        : position(p), rotation(r), scale(s) , model(m) {}
};

struct Volume {
    virtual bool isOnFrustum(const Frustum& camFrustum, const Transform& modelTransform) const = 0;
};

struct Sphere : public Volume {
    glm::vec3 center{ 0.f, 0.f, 0.f };
    float radius{ 0.f };

	Sphere(glm::vec3 c, float r)
		: center(c) , radius(r) {}

	bool isOnFrustum(const Frustum& camFrustum, const Transform& transform) const final {
    //Get global scale is computed by doing the magnitude of
    //X, Y and Z model matrix's column.
    const glm::vec3 globalScale = transform.scale;

    //Get our global center with process it with the global model matrix of our transform
    const glm::vec3 globalCenter = { transform.model * glm::vec4(center, 1.f) };

    //To wrap correctly our shape, we need the maximum scale scalar.
    const float maxScale = std::max(std::max(globalScale.x, globalScale.y), globalScale.z);

    //Max scale is assuming for the diameter. So, we need the half to apply it to our radius
    Sphere globalSphere(globalCenter, radius * (maxScale * 0.5f));

    //Check Firstly the result that have the most chance
    //to faillure to avoid to call all functions.
    return (globalSphere.isOnOrForwardPlane(camFrustum.leftFace) &&
        globalSphere.isOnOrForwardPlane(camFrustum.rightFace) &&
        globalSphere.isOnOrForwardPlane(camFrustum.farFace) &&
        globalSphere.isOnOrForwardPlane(camFrustum.nearFace) &&
        globalSphere.isOnOrForwardPlane(camFrustum.topFace) &&
        globalSphere.isOnOrForwardPlane(camFrustum.bottomFace));
	};

	bool isOnOrForwardPlane(const Plane& plane) const{
    return plane.getSignedDistanceToPlane(center) > -radius;
	}
    
};


   



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

	if(extraThreads < 1) joinableThreads[0] = false;

	


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

	Shader text1Shader("../resources/shaders/text.vert", "../resources/shaders/text.frag");
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
	if (FT_New_Face(ft, "../resources/fonts/mainFont.ttf", 0, &face1))
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
	Shader shaderProgramBlocks("../resources/shaders/tiles.vert", "../resources/shaders/tiles.frag");
	Shader skyboxShader("../resources/shaders/skybox.vert", "../resources/shaders/skybox.frag");
	Shader lightObjectShader("../resources/shaders/lights.vert", "../resources/shaders/lights.frag");
	Shader lightOutlineShader("../resources/shaders/lights.vert", "../resources/shaders/outline.frag");
	Shader foliageShader("../resources/shaders/foliage.vert", "../resources/shaders/tiles.frag");
	Shader debugShader("../resources/shaders/debug.vert", "../resources/shaders/debug.frag");
	Shader godRaysShader("../resources/shaders/rays.vert", "../resources/shaders/rays.frag");

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
		"../resources/textures/skybox/right.png",
		"../resources/textures/skybox/left.png",
		"../resources/textures/skybox/up.png",
		"../resources/textures/skybox/down.png",
		"../resources/textures/skybox/front.png",
		"../resources/textures/skybox/back.png",
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

	chunk = new Chunk(0.0f, 10.0f, 0.0f);
	worker1chunk = new Chunk(0.0f, 10.0f, 0.0f);

	meshes = new Mesh[meshesAmount];
	setSpawn(0, 0);

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

	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	glGenTextures(1, &textureColorbuffer);
	glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, window_width, window_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);

	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, window_width, window_height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//textures
	stbi_set_flip_vertically_on_load(true);
	const char* atlasLevels[] = {
		"../resources/textures/atlas.png",
		"../resources/textures/atlas(1).png",
		"../resources/textures/atlas(2).png",
		"../resources/textures/atlas(3).png",
		"../resources/textures/atlas(4).png",
		"../resources/textures/atlas(5).png",
	};

	const char* atlasSpecularLevels[] = {
		"../resources/textures/atlasSpecular.png",
		"../resources/textures/atlas(1).png",
		"../resources/textures/atlas(2).png",
		"../resources/textures/atlas(3).png",
		"../resources/textures/atlas(4).png",
		"../resources/textures/atlas(5).png"
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
	short threadTurn = 0;
	unsigned int meshBuffersProgressIndex;
	chunkCoords chunksOrderSaver;

	Frustum frus = createFrustumFromCamera(camera, float(window_width) / float(window_height), glm::radians(camera.Zoom), 0.1f, 1000.f);

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

		if (debug.showChunkBorders) {
			glDisable(GL_CULL_FACE);
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);\
			debugShader.use();
			debugShader.setMat4("pv", projection * view);
			glBindVertexArray(debugVAO);

			for (unsigned int i = 0; i < meshesAmount; i++) {
				//if(sqrt((meshes[i].X - camera.Position.x) * (meshes[i].X - camera.Position.x) + (meshes[i].Y - camera.Position.y) * (meshes[i].Y - camera.Position.y) + (meshes[i].Z - camera.Position.z) * (meshes[i].Z - camera.Position.z)) < 45){
					model = glm::mat4(1.0f);
					model = glm::translate(model, glm::vec3(meshes[i].X, meshes[i].Y, meshes[i].Z));
					if(meshes[i].length == 0){
						debugShader.setBool("skipped", true);
					}else{
						debugShader.setBool("skipped", false);
					}
					debugShader.setMat4("model", model);
					glDrawArrays(GL_TRIANGLES, 0, 36);
				//}
			}
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);\
			glEnable(GL_CULL_FACE);
		}

		shaderProgramBlocks.use();
		shaderProgramBlocks.setMat4("pv", projection * view);
		shaderProgramBlocks.setVec3("viewPos", camera.Position);

		shaderProgramBlocks.setVec3("dirLight[0].direction", 0.251f, 0.267f, 0.2f);
		shaderProgramBlocks.setVec3("dirLight[0].ambient", 0.1f, 0.1f, 0.1f);
		shaderProgramBlocks.setVec3("dirLight[0].diffuse", 0.85f, 0.8f, 0.9f);
		shaderProgramBlocks.setVec3("dirLight[0].specular", 0.3f, 0.3f, 0.3f);

		shaderProgramBlocks.setVec3("dirLight[1].direction", 0.051f, 0.067f, 0.0f);
		shaderProgramBlocks.setVec3("dirLight[1].ambient", 0.1f, 0.1f, 0.1f);
		shaderProgramBlocks.setVec3("dirLight[1].diffuse", 0.8f, 0.8f, 0.8f);
		shaderProgramBlocks.setVec3("dirLight[1].specular", 0.7f, 0.7f, 0.7f);

		shaderProgramBlocks.setVec3("dirLight[2].direction", -0.149, -0.133f, -0.2f);
		shaderProgramBlocks.setVec3("dirLight[2].ambient", 0.1f, 0.1f, 0.1f);
		shaderProgramBlocks.setVec3("dirLight[2].diffuse", 0.95f, 0.8f, 0.85f);
		shaderProgramBlocks.setVec3("dirLight[2].specular", 0.3f, 0.3f, 0.3f);

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

		/*shaderProgramBlocks.setVec3("spotLight.position", camera.Position);
		shaderProgramBlocks.setVec3("spotLight.direction", camera.Front);
		shaderProgramBlocks.setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
		shaderProgramBlocks.setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
		shaderProgramBlocks.setVec3("spotLight.specular", 3.0f, 3.0f, 3.0f);
		shaderProgramBlocks.setFloat("spotLight.constant", 0.2f);
		shaderProgramBlocks.setFloat("spotLight.linear", 0.09f);
		shaderProgramBlocks.setFloat("spotLight.quadratic", 0.032f);
		shaderProgramBlocks.setFloat("spotLight.cutOff", glm::cos(glm::radians(15.0f)));
		shaderProgramBlocks.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(17.5f)));*/

		if(held(BUTTON_B)) setSpawn(camera.Position.x, camera.Position.z);

		for (unsigned int i = 0; i < realMeshes.size(); i++) {
			model = glm::mat4(1.0f);
			model = glm::translate(model, glm::vec3(meshes[realMeshes[i]].X, meshes[realMeshes[i]].Y, meshes[realMeshes[i]].Z));
			glBindVertexArray(meshes[realMeshes[i]].VAO);
			shaderProgramBlocks.setFloat("LODstep", float(meshes[realMeshes[i]].distanceI));
			shaderProgramBlocks.setMat4("model", model);
			shaderProgramBlocks.setFloat("material.shininess", 32.f);
			glDrawArrays(GL_TRIANGLES, 0, meshes[realMeshes[i]].length);
		}

		/*frus = createFrustumFromCamera(camera, float(window_width) / float(window_height), glm::radians(camera.Zoom), 0.1f, 1000.f);
		for (unsigned int i = 0; i < realMeshes.size(); i++) {
			Sphere check(glm::vec3(meshes[realMeshes[i]].X, meshes[realMeshes[i]].Y, meshes[realMeshes[i]].Z), 20);
			model = glm::mat4(1.0f);
			model = glm::translate(model, glm::vec3(meshes[realMeshes[i]].X, meshes[realMeshes[i]].Y, meshes[realMeshes[i]].Z));
			if (check.isOnFrustum(frus, {
				glm::vec3(meshes[realMeshes[i]].X, meshes[realMeshes[i]].Y, meshes[realMeshes[i]].Z),
				glm::vec3(0.0f),
				glm::vec3(1.0f),
				model
			})){
				glBindVertexArray(meshes[realMeshes[i]].VAO);
				shaderProgramBlocks.setFloat("LODstep", float(meshes[realMeshes[i]].distanceI));
				shaderProgramBlocks.setMat4("model", model);
				shaderProgramBlocks.setFloat("material.shininess", 32.f);
				//glDrawElements(GL_TRIANGLES, meshes[i].indicesAmount, GL_UNSIGNED_INT, 0);
				glDrawArrays(GL_TRIANGLES, 0, meshes[realMeshes[i]].length);
			}
		}*/

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
		godRaysShader.setMat4("view", view);
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
		if (pressed(BUTTON_8) && generate == false) {
			bytesFromMeshes = 0;
			unsigned int count = 0;
			meshesAmount = 0;
			totalChunks = 0;
			generatePos = newPos;
			threadTurn = 0;
			neededThreadGeneratedChunks = 0;
			if(extraThreads == 1){
				neededChunks.clear();
			}

			totalChunks = sliderTester1;
			
			completedChunks = 0;
			meshBuffersProgressIndex = 0;
			createAllMeshes = false;
			makeChunksOrder = true;
			worker1Finished = false;
		}

		if(createAllMeshes){
			delete[] meshes;
			meshes = new Mesh[meshesAmount];
			createAllMeshes = false;
		}
		if(neededMeshBuffers.size() > 0){
			for(int i = 0; i < 500; i++){
				if(meshBuffersProgressIndex >= neededMeshBuffers.size() - 1) goto buffersExit;
				meshes[realMeshes[meshBuffersProgressIndex++]].updateBuffers();
			}
		}
		buffersExit:

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		debugInputs();
		ImGui::Begin("ImGui window");
		ImGui::SliderFloat("value", &change, -100.0f, 100.0f, "%.3f", 0);
		ImGui::Text("FPS: = %i", debug.FPS);
		ImGui::Text("X: = %f, Y: %f, Z: %f", camera.Position.x, camera.Position.y, camera.Position.z);
		ImGui::Text("Memory from meshes, bytes: = %.f, KB: %.f, MB: %.f", bytesFromMeshes, bytesFromMeshes / 1000, bytesFromMeshes / 1000000);
		ImGui::SliderFloat("Render Distance", &sliderTester1, 1, 100, "%.f", 0);

		ImGui::End();
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
		active = true;
	}
	
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	joinableThreads[0] = false;
	worker1.join();
	delete chunk;
	delete worker1chunk;
	delete[] meshes;
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
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, window_width, window_height);
	glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, window_width, window_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
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
		process_button(BUTTON_B, 66);
		process_button(BUTTON_C, 67);
		process_button(BUTTON_E, 69);
		process_button(BUTTON_F, 70);
		process_button(BUTTON_G, 71);
		process_button(BUTTON_H, 72);
		process_button(BUTTON_I, 73);
		process_button(BUTTON_J, 74);
		process_button(BUTTON_K, 75);
		process_button(BUTTON_L, 76);
		process_button(BUTTON_M, 77);
		process_button(BUTTON_N, 78);
		process_button(BUTTON_O, 79);
		process_button(BUTTON_P, 80);
		process_button(BUTTON_Q, 81);
		process_button(BUTTON_R, 82);
		process_button(BUTTON_T, 84);
		process_button(BUTTON_U, 85);
		process_button(BUTTON_V, 86);
		process_button(BUTTON_X, 88);
		process_button(BUTTON_Y, 89);
		process_button(BUTTON_Z, 90);
		process_button(BUTTON_LEFT_CTRL, 341);
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

void timeBenchmark(bool stop){
	static float oldTime;
	if(stop){
		std::cout << "Total time: " << float(glfwGetTime()) - oldTime << '\n';
	}else{
		oldTime = float(glfwGetTime());
		std::cout << "Starting benchmark\n";
	}
}

void chunker1(){
	while(active == false){

	}
	while(true){
	if(!worker1Finished){
		if(makeChunksOrder){
			timeBenchmark(0);
			generate = true;

			for (int y = 0; y < std::clamp(totalChunks, 0, 40); y++) {
				for (int z = 0; z < totalChunks; z++) {
					for (int x = 0; x < totalChunks; x++) {
						if (sqrt((x * x) + (y * y) * 4 + (z * z)) <= totalChunks) {
							if(joinableThreads[0] == false) break;
							chunkCoords threadChunk;
							threadChunk.x = generatePos.x + x * 10.0f;
							threadChunk.y = generatePos.y + (y - 1) * 10.0f;
							threadChunk.z = generatePos.z + z * 10.0f;
							threadChunk.index = completedChunks;
							neededChunks.push_back(threadChunk);
							completedChunks++;
							meshesAmount++;
							x *= -1;
							if(x < 0) x--;
						}
					}
					z *= -1;
					if(z < 0) z--;
				}
				y *= -1;
				if(y < 0) y--;
			}
			std::reverse(neededChunks.begin(), neededChunks.end());

			/*
			std::unordered_map<glm::vec3, bool, vec3Hash> repeatingChunks;
			for(int i = 1; i < totalChunks; i+= (i > 6) * (int(i + 5 < totalChunks) + int(i + 4 < totalChunks) + int(i + 3 < totalChunks)) + int(i + 2 < totalChunks) + 1){
				for (int y = i * -1; y < i; y++) {
					for (int z = i * -1; z < i; z++) {
						for (int x = i * -1; x < i; x++) {
							if(joinableThreads[0] == false) break;
							if(repeatingChunks.find(glm::vec3(x, y, z)) == repeatingChunks.end()){
								if (sqrt((x * x) + (y * y) * 4 + (z * z)) <= i) {
									repeatingChunks.insert({glm::vec3(x, y, z), true});
									chunkCoords threadChunk;
									threadChunk.x = generatePos.x + x * 10.0f;
									threadChunk.y = generatePos.y + y * 10.0f;
									threadChunk.z = generatePos.z + z * 10.0f;
									threadChunk.index = completedChunks;
									neededChunks.insert(neededChunks.begin(), threadChunk);
									completedChunks++;
									meshesAmount++;
								}
							}
						}
					}
				}
			}
			*/
			makeChunksOrder = false;
			neededThreadGeneratedChunks = meshesAmount;
			createAllMeshes = true;
			realMeshes.clear();
			while(createAllMeshes){

			}
			totalChunksGenerated = 0;
			timeBenchmark(1);
		}else{
			timeBenchmark(0);
			while(neededChunks.size() > 0){
				int index = neededChunks.size() - 1;
				worker1chunk->create(neededChunks[index].x, neededChunks[index].y, neededChunks[index].z);
				meshes[neededChunks[index].index].fillChunk(*worker1chunk);
				if(meshes[neededChunks[index].index].length > 0){
					neededMeshBuffers.push_back(neededChunks[index].index);
					realMeshes.push_back(neededChunks[index].index);
				}
				neededChunks.erase(neededChunks.begin() + index);
				static int lastPercent = -1;
				int percent = int((float(totalChunksGenerated) / neededThreadGeneratedChunks) * 100);
				if (percent != lastPercent) {
				    std::cout << percent << "%\n";
				    lastPercent = percent;
				}	
				if(joinableThreads[0] == false) break;
			}
			worker1Finished = true;
			generate = false;
			neededMeshBuffers.clear();
			timeBenchmark(1);
		}
		
	}
	if(joinableThreads[0] == false) break;
	}
	std::cout << "\nWORKER1 THREAD ENDED";
}

void setSpawn(float x, float z){
	x = round(int(x) / 10) * 10.0f;
	z = round(int(z) / 10) * 10.0f;
    std::vector<float> meshHeights;
	if(meshesAmount > 0){
 		for(int i = 0; i < meshesAmount; i++){
        	if(meshes[i].X == x && meshes[i].Z == z && meshes[i].length > 0) meshHeights.push_back(meshes[i].Y);
    	}
		auto autoMax = std::max_element(meshHeights.begin(), meshHeights.end());
		int max = *autoMax;
    	camera.Position.y = chunk->calcNoise(camera.Position.x, camera.Position.z, 1) + 15;
	}
}