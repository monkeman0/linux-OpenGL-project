#include <iostream>
#include <cmath>
#include <string>
#include <time.h>
#include <vector>
#include <algorithm>
#include <memory>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>
#include <filesystem>
#include <ios>
#ifdef _WIN32
#   include <windows.h>
#   include <psapi.h>        // GetProcessMemoryInfo
#else
#   include <unistd.h>
#   include <malloc.h>
#   include <sys/sysinfo.h>
#   include <sys/resource.h>
#endif

inline void trim_heap() {
#if defined(__GLIBC__)
    malloc_trim(0);
#endif
}


#include "glad.h"
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

//font loader
#include <ft2build.h>
#include FT_FREETYPE_H

std::mutex realMeshesQueueMutex;
std::condition_variable realMeshesQueueCV;

float sliderTester1 = 45;
float sliderTester2 = 32;
float sliderTester3 = 0.f;
float t1, t2, t3, t4, t5 = 0.f;
int startingWindow_width = 0;
int startingWindow_height = 0;


unsigned int bytesFromChunks = 0;
std::mutex chunksSearchMutex;

#include "tileHandling.h"
#include "classes.h"
#include "cameraClass.h"

std::unordered_map<glm::vec3, unsigned int> chunksSearch;
std::unordered_map<glm::vec2, float[32*32]> noiseSearch;

double pi = 3.141592653589793;
float deltaTime = 0.0f;
float lastTime = 0.0f;
std::atomic<size_t> freeRam{0};
std::atomic<size_t> rss{0};

int window_width, window_height;
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::mat4 projection;

#define debugSpeeds()\
if (debug.moveMode == '1') {\
	debug.speed = 2.5f;\
}\
else if(debug.moveMode == '2'){\
	debug.speed = 160.0f;\
}else{\
	debug.speed = 24.0f;\
}\
	camera.MovementSpeed = debug.speed;\

#define debugInputs()\
if (pressed(BUTTON_B))	debug.showFrustum = !debug.showFrustum;\
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
if (pressed(BUTTON_F11)){\
	debug.fullscreen = !debug.fullscreen;\
	glfwSetWindowAttrib(window, GLFW_DECORATED, !debug.fullscreen);\
	if(debug.fullscreen) {\
		glfwSetWindowMonitor(window, glfwGetPrimaryMonitor(), 0, 0, startingWindow_width, startingWindow_height, 60);\
		glfwSetWindowSize(window, startingWindow_width, startingWindow_height);\
	} else {\
		glfwSetWindowMonitor(window, nullptr, 0, 0, startingWindow_width, startingWindow_height, 60);\
		glfwSetWindowSize(window, static_cast<int>(startingWindow_width * 0.8f), static_cast<int>(startingWindow_height * 0.8f));\
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

//framebuffers creation
frameBuffer horizontalBlurBuffer;
frameBuffer verticalBlurBuffer;
frameBuffer plainTerrainBuffer;
frameBuffer darkHorizontalBlurBuffer;
frameBuffer darkVerticalBlurBuffer;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void window_focus_callback(GLFWwindow* window, int focused);
void mouse_button_callback(GLFWwindow* window, int button, int action, int bits);
void RenderText(Shader& s, std::string text, float x, float y, float scale, glm::vec3 color);
void timeBenchmark(bool stop);
void setSpawn(float x, float z);
void chunker1();
void loadTiles();
void get_free_ram();
void get_used_ram();

ThreadSafeVector<Chunk> chunks;
ThreadSafeVector<Mesh> meshes;

int totalChunks = 0;
std::atomic<bool> generate{false};
std::atomic<bool> worker1Finished{true};
unsigned int completedChunks = 0;
std::atomic<bool> clearingChunks{false};
std::queue<unsigned int> realMeshesQueue;

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
std::atomic<bool> makeChunksOrder{0};

struct Plane {
	glm::vec3 normal;
	float distance;

	Plane() = default;

	Plane(const glm::vec3& p1, const glm::vec3& norm)
		: normal(glm::normalize(norm)),
		distance(glm::dot(normal, p1))
	{
	}

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

    float halfVSide = zFar * tanf(fovY * 0.5f);
    float halfHSide = halfVSide * aspect;

    glm::vec3 frontMultFar = cam.Front * zFar;

    // The Near and Far planes
    frustum.nearFace = Plane(cam.Position + cam.Front * zNear, cam.Front);
    frustum.farFace = Plane(cam.Position + frontMultFar, -cam.Front);

    // Top plane: Cross product of 'Right' and the vector pointing to the top edge
    glm::vec3 topNormal = glm::cross(cam.Right, glm::normalize(frontMultFar - cam.Up * halfVSide));
    frustum.topFace = Plane(cam.Position, topNormal);

    // Bottom plane: Cross product of 'Right' and the vector pointing to the bottom edge
    glm::vec3 bottomNormal = glm::cross(glm::normalize(frontMultFar + cam.Up * halfVSide), cam.Right);
    frustum.bottomFace = Plane(cam.Position, bottomNormal);

    // Left plane: Cross product of the vector pointing to the left edge and 'Up'
    glm::vec3 leftNormal = glm::cross(glm::normalize(frontMultFar - cam.Right * halfHSide), cam.Up);
    frustum.leftFace = Plane(cam.Position, leftNormal);

    // Right plane: Cross product of 'Up' and the vector pointing to the right edge
    glm::vec3 rightNormal = glm::cross(cam.Up, glm::normalize(frontMultFar + cam.Right * halfHSide));
    frustum.rightFace = Plane(cam.Position, rightNormal);

    return frustum;
}

struct Transform {
	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 scale;
	glm::mat4 model;

	Transform(glm::vec3 p, glm::vec3 r, glm::vec3 s, glm::mat4 m)
		: position(p), rotation(r), scale(s), model(m) {
	}
};

struct Volume {
	virtual bool isOnFrustum(const Frustum& camFrustum, const Transform& modelTransform) const = 0;
};

struct AABB : public Volume {
    glm::vec3 center{ 0.f, 0.f, 0.f };
    glm::vec3 extents{ 16.f, 16.f, 16.f }; // Half-dimensions (width/2, height/2, depth/2)

    AABB(const glm::vec3& inCenter, float halfX, float halfY, float halfZ)
        : center(inCenter), extents(halfX, halfY, halfZ) {}

    bool isOnOrForwardPlane(const Plane& plane) const {
        // Compute the projection interval radius of the AABB onto the plane normal
        const float r = extents.x * std::abs(plane.normal.x) +
                        extents.y * std::abs(plane.normal.y) +
                        extents.z * std::abs(plane.normal.z);

        return -r <= plane.getSignedDistanceToPlane(center);
    }

    bool isOnFrustum(const Frustum& camFrustum, const Transform& transform) const final {
        // Get our global center by translating it with the model matrix
        const glm::vec3 globalCenter = glm::vec3(transform.model * glm::vec4(center, 1.f));

        // Since it's a voxel chunk, we assume it is never rotated, only scaled/translated.
        // This keeps the AABB perfectly axis-aligned in world space!
        const glm::vec3 globalExtents = extents * transform.scale;

        AABB globalAABB(globalCenter, globalExtents.x, globalExtents.y, globalExtents.z);

        // Check against all 6 planes
        return (globalAABB.isOnOrForwardPlane(camFrustum.leftFace) &&
                globalAABB.isOnOrForwardPlane(camFrustum.rightFace) &&
                globalAABB.isOnOrForwardPlane(camFrustum.nearFace) &&
                globalAABB.isOnOrForwardPlane(camFrustum.farFace) &&
                globalAABB.isOnOrForwardPlane(camFrustum.topFace) &&
                globalAABB.isOnOrForwardPlane(camFrustum.bottomFace));
    }
};

std::vector<glm::vec3> getFrustumCornersWorldSpace(const glm::mat4& view, const glm::mat4& projection) {
    // Inverse of the combined view-projection matrix
    glm::mat4 invVP = glm::inverse(projection * view);

    // The 8 corners of the NDC cube
    glm::vec4 ndcCorners[8] = {
        // Near plane
        {-1.0f, -1.0f, -1.0f, 1.0f}, { 1.0f, -1.0f, -1.0f, 1.0f}, 
        { 1.0f,  1.0f, -1.0f, 1.0f}, {-1.0f,  1.0f, -1.0f, 1.0f},
        // Far plane
        {-1.0f, -1.0f,  1.0f, 1.0f}, { 1.0f, -1.0f,  1.0f, 1.0f}, 
        { 1.0f,  1.0f,  1.0f, 1.0f}, {-1.0f,  1.0f,  1.0f, 1.0f}
    };

    std::vector<glm::vec3> worldCorners;
    for (int i = 0; i < 8; i++) {
        glm::vec4 pt = invVP * ndcCorners[i];
        // Perspective divide is crucial to get real world coordinates!
        worldCorners.push_back(glm::vec3(pt) / pt.w);
    }

    return worldCorners;
}


int main() {
	srand(static_cast<unsigned int>(time(NULL)));
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_DECORATED, GL_TRUE);
	const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	window_width = mode->width;
	window_height = mode->height;
	startingWindow_height = window_height;
	startingWindow_width = window_width;
	debug.fullscreen = true;
	short extraThreads = std::thread::hardware_concurrency() - 1;

	GLFWwindow* window = glfwCreateWindow(window_width, window_height, "openGL window", glfwGetPrimaryMonitor(), NULL);
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
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetWindowFocusCallback(window, window_focus_callback);


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
	ImGui_ImplOpenGL3_Init("#version 460 core");

	if (extraThreads < 1) joinableThreads[0] = false;
	size_t totalRam = 0;
	{
		struct sysinfo si;
		if (sysinfo(&si) == 0) totalRam = (si.totalram * si.mem_unit) / (1024 * 1024);
	}

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
	Shader horizontalBlurShader("../resources/shaders/simple.vert", "../resources/shaders/horizontalBlur.frag");
	Shader verticalBlurShader("../resources/shaders/simple.vert", "../resources/shaders/verticalBlur.frag");
	Shader frustumShader("../resources/shaders/frustum.vert", "../resources/shaders/frustum.frag");

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

	Chunk::noiseInit();
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
	glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, 1 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	unsigned int frusVAO, frusVBO, frusEBO;
	glGenVertexArrays(1, &frusVAO);
	glGenBuffers(1, &frusVBO);
	glGenBuffers(1, &frusEBO);
	glBindVertexArray(frusVAO);
	glBindBuffer(GL_ARRAY_BUFFER, frusVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float), &debugVertices, GL_DYNAMIC_DRAW);
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

	//framebuffers setup
	plainTerrainBuffer.build(window_width, window_height);
	horizontalBlurBuffer.build(window_width, window_height);
	verticalBlurBuffer.build(window_width, window_height);
	darkHorizontalBlurBuffer.build(window_width, window_height);
	darkVerticalBlurBuffer.build(window_width, window_height);

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
	projection = glm::perspective(glm::radians(camera.Zoom), float(window_width) / float(window_height), 0.1f, 20000.0f); // last float is frustum distance

	shaderProgramBlocks.use();
	shaderProgramBlocks.setInt("material.diffuse", 1);
	shaderProgramBlocks.setInt("material.specular", 2);
	debugShader.use();
	debugShader.setInt("aTexture", 1);
	foliageShader.use();
	foliageShader.setInt("material.diffuse", 1);
	foliageShader.setInt("material.specular", 2);
	horizontalBlurShader.use();
	horizontalBlurShader.setInt("screenTexture", 0);
	verticalBlurShader.use();
	verticalBlurShader.setInt("screenTexture", 0);

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
	Frustum frus;
	bool frustumDebugReady = false;

	while (!glfwWindowShouldClose(window)) {

		//get input changed bools ready to recieve input
		for (int i = 0; i < BUTTON_COUNT; i++) {
			input.buttons[i].is_changed = false;
		}
		//get delta time
		deltaTime = float(glfwGetTime()) - lastTime;
		lastTime = float(glfwGetTime());
		glfwPollEvents();

		glm::vec3 prevPos = glm::vec3(round(camera.Position.x / 32) * 32.0f, round(camera.Position.y / 32) * 32.0f, round(camera.Position.z / 32) * 32.0f);
		//std::cout << "X: " << prevPos.x << " Y: " << prevPos.y << " Z: " << prevPos.z << '\n';

		if (held(BUTTON_W))  camera.ProcessKeyboard(FORWARD, deltaTime);
		if (held(BUTTON_S))  camera.ProcessKeyboard(BACKWARD, deltaTime);
		if (held(BUTTON_A))  camera.ProcessKeyboard(LEFT, deltaTime);
		if (held(BUTTON_D))	 camera.ProcessKeyboard(RIGHT, deltaTime);
		if (held(BUTTON_SPACE))	camera.ProcessKeyboard(UP, deltaTime);
		if (held(BUTTON_LEFTSHIFT)) camera.ProcessKeyboard(DOWN, deltaTime);
		camera.Roll *= (abs((atan(deltaTime * 10.0f) - 95.f) * 0.01f) * (abs(camera.Roll) > 0.05f));
		camera.updateCameraVectors();

		glm::vec3 newPos = glm::vec3(round(camera.Position.x / 32) * 32.0f, round(camera.Position.y / 32) * 32.0f, round(camera.Position.z / 32) * 32.0f);

		if (newPos != prevPos) {

		} //loadNewChunks(sliderTester1, newPos);
		glm::mat4 model = glm::mat4(1.0f);
		glm::mat4 view = camera.GetViewMatrix();

		plainTerrainBuffer.drawTo();

		glEnable(GL_DEPTH_TEST);
		//glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		glStencilMask(0xFF);
		glStencilFunc(GL_ALWAYS, 0, 0xFF);



			if (debug.showChunkBorders) {
				glDisable(GL_CULL_FACE);
				
				debugShader.use();
				debugShader.setMat4("pv", projection * view);
				glBindVertexArray(debugVAO);
				size_t numChunks = chunks.size();
				for (size_t i = 0; i < numChunks; i++) {
					if (clearingChunks.load()) continue;
					std::shared_ptr<Chunk> chunk = chunks.get(i);
					std::lock_guard<std::mutex> chunkLock(chunk->chunkMtx);

					model = glm::mat4(1.0f);
					model = glm::translate(model, glm::vec3(chunk->X, chunk->Y, chunk->Z));

					Transform chunkTransform(
		                glm::vec3(chunk->X, chunk->Y, chunk->Z),
		                glm::vec3(0.0f),
		                glm::vec3(1.0f),
		                model
		            );
			
					AABB check(glm::vec3(16.0f, 16.0f, 16.0f), 16.0f, 16.0f, 16.0f);

		            if (check.isOnFrustum(frus, chunkTransform)) {
						if(chunk->solid || chunk->empty){
							debugShader.setBool("skipped", true);
						}else{
							debugShader.setBool("skipped", false);
						}
						debugShader.setMat4("model", model);
						glDrawArrays(GL_LINES, 0, 24);
					}
					//if(chunk && sqrt((chunk->X - camera.Position.x) * (chunk->X - camera.Position.x) + (chunk->Y - camera.Position.y) * (chunk->Y - camera.Position.y) + (chunk->Z - camera.Position.z) * (chunk->Z - camera.Position.z)) < 150){
						
					//}
				}
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

		if(!debug.showFrustum) frus = createFrustumFromCamera(camera, float(window_width) / float(window_height), glm::radians(camera.Zoom), 0.1f, 20000.f);

		{
		    size_t numMeshes = meshes.size();
		    for (size_t i = 0; i < numMeshes; i++) {
		        if (clearingChunks.load()) continue;
			
		        std::shared_ptr<Mesh> mesh = meshes.get(i);
		        if (mesh && mesh->VAO != 0 && mesh->readable) {
				
		            glm::mat4 model = glm::mat4(1.0f);
		            model = glm::translate(model, glm::vec3(mesh->X, mesh->Y, mesh->Z));
				
		            Transform meshTransform(
		                glm::vec3(mesh->X, mesh->Y, mesh->Z),
		                glm::vec3(0.0f),
		                glm::vec3(1.0f),
		                model
		            );
			
					AABB check(glm::vec3(16.0f, 16.0f, 16.0f), 16.0f, 16.0f, 16.0f);

		            if (check.isOnFrustum(frus, meshTransform)) {
		                glBindVertexArray(mesh->VAO);
		                shaderProgramBlocks.setFloat("LODstep", float(mesh->distanceI));
		                shaderProgramBlocks.setMat4("model", model);
		                shaderProgramBlocks.setFloat("material.shininess", 32.f);
		                glDrawArrays(GL_TRIANGLES, 0, mesh->vertices.size());
		            }
				   	/*glBindVertexArray(mesh->VAO);
		            shaderProgramBlocks.setFloat("LODstep", float(mesh->distanceI));
		            shaderProgramBlocks.setMat4("model", model);
		            shaderProgramBlocks.setFloat("material.shininess", 32.f);
		            glDrawArrays(GL_TRIANGLES, 0, mesh->vertices.size());*/
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
		glBindVertexArray(screenQuadVAO);

		//draw to buffers
		horizontalBlurShader.use();
		//horizontalBlurBuffer.drawTo(); replace lower line with this when done
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		horizontalBlurShader.setMat4("view", view);
		horizontalBlurShader.setInt("post", 0);
		//glActiveTexture(GL_TEXTURE0);
		plainTerrainBuffer.readFrom();
		glDrawArrays(GL_TRIANGLES, 0, 6);

		/*
		verticalBlurBuffer.drawTo();
		verticalBlurShader.use();
		verticalBlurShader.setMat4("view", view);
		verticalBlurShader.setInt("post", 1);
		glActiveTexture(GL_TEXTURE0);
		horizontalBlurBuffer.readFrom();
		glDrawArrays(GL_TRIANGLES, 0, 6);

		darkHorizontalBlurBuffer.drawTo();
		horizontalBlurShader.use();
		horizontalBlurShader.setInt("post", 2);
		glActiveTexture(GL_TEXTURE0);
		plainTerrainBuffer.readFrom();
		glDrawArrays(GL_TRIANGLES, 0, 6);

		darkVerticalBlurBuffer.drawTo();
		verticalBlurShader.use();
		verticalBlurShader.setInt("post", 2);
		glActiveTexture(GL_TEXTURE0);
		darkHorizontalBlurBuffer.readFrom();
		glDrawArrays(GL_TRIANGLES, 0, 6);

		//draw to screen after populating buffers
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		verticalBlurShader.use();
		verticalBlurShader.setInt("post", 1);
		glActiveTexture(GL_TEXTURE0);
		verticalBlurBuffer.readFrom();
		glDrawArrays(GL_TRIANGLES, 0, 6);
		
		verticalBlurShader.use();
		verticalBlurShader.setInt("post", 2);
		glActiveTexture(GL_TEXTURE0);
		darkVerticalBlurBuffer.readFrom();
		glDrawArrays(GL_TRIANGLES, 0, 6);*/


		if(debug.showFrustum){
			glBindVertexArray(frusVAO);
			frustumShader.use();
			if(pressed(BUTTON_F)){
				frus = createFrustumFromCamera(camera, float(window_width) / float(window_height), glm::radians(camera.Zoom), 0.1f, 20000.f);
				float aspect = float(window_width) / float(window_height);
				glm::mat4 debugProj = glm::perspective(glm::radians(camera.Zoom), aspect, 0.1f, 1000.0f);	
				// 2. Use camera.GetViewMatrix() and our debugProj to get the corners
				std::vector<glm::vec3> cornersI = getFrustumCornersWorldSpace(camera.GetViewMatrix(), debugProj);	
				float corners[24] = {0.0f};
				for (int i = 0; i < 8; i++) {
				    corners[i * 3 + 0] = cornersI[i].x;
				    corners[i * 3 + 1] = cornersI[i].y;
				    corners[i * 3 + 2] = cornersI[i].z;
				}
			
				unsigned int frustumIndices[] = {
    				// Near plane (Front)
    				0, 1, 2,   0, 2, 3,
    				// Far plane (Back)
    				4, 7, 6,   4, 6, 5,
    				// Left plane
    				0, 3, 7,   0, 7, 4,
    				// Right plane
    				1, 5, 6,   1, 6, 2,
    				// Top plane
    				3, 2, 6,   3, 6, 7,
    				// Bottom plane
    				0, 4, 5,   0, 5, 1
				};
			
				glBindBuffer(GL_ARRAY_BUFFER, frusVBO);
				glBufferData(GL_ARRAY_BUFFER, sizeof(corners), corners, GL_DYNAMIC_DRAW);	
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, frusEBO);
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(frustumIndices), frustumIndices, GL_DYNAMIC_DRAW);	
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
				glEnableVertexAttribArray(0);	
				frustumDebugReady = true;
			}
			frustumShader.setMat4("view", camera.GetViewMatrix()); 
    		frustumShader.setMat4("projection", projection);
			glm::mat4 model = glm::mat4(1.0f);
    		frustumShader.setMat4("model", model);
			glDisable(GL_CULL_FACE);
			if(frustumDebugReady) glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
			glEnable(GL_CULL_FACE);
		}

		glEnable(GL_DEPTH_TEST);

		

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

		if (pressed(BUTTON_8) && generate.load() == false) {
			unsigned int count = 0;
			totalChunks = 0;
			generatePos = newPos;
			if (extraThreads == 1) {
			}

			totalChunks = sliderTester1;

			completedChunks = 0;
			worker1Finished = false;
			makeChunksOrder = true;
		}

		{
			std::unique_lock<std::mutex> lk(realMeshesQueueMutex);
			int processed = 0;
			while (!realMeshesQueue.empty() && processed < 250) {
				unsigned int index = realMeshesQueue.front();
				realMeshesQueue.pop();
				lk.unlock();

				std::shared_ptr<Mesh> mesh = meshes.get(index);
				if (mesh) {
					std::lock_guard<std::mutex> meshLock(mesh->meshMtx);
					if (mesh->VAO == 0) mesh->init();
					mesh->updateBuffers();
					mesh->readable = true;
				}
				mesh.reset();
				lk.lock();

				processed++;
			}
		}

		if(pressed(BUTTON_J)){
			std::cout << sizeof(chunksSearch) + (chunksSearch.bucket_count() * sizeof(void*)) + (chunksSearch.size() * (sizeof(decltype(chunksSearch)::value_type) + 16)) << std::endl;
		}

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		debugInputs();
		ImGui::Begin("ImGui window");
		//ImGui::SliderFloat("value", &change, -100.0f, 100.0f, "%.3f", 0);
		ImGui::Text("FPS: = %i", debug.FPS);
		ImGui::Text("X: = %f, Y: %f, Z: %f", camera.Position.x, camera.Position.y, camera.Position.z);
		get_free_ram();
		get_used_ram();
		ImGui::Text("Memory used in MB: %llu, GB: %llu", (unsigned long long)rss.load(), (unsigned long long)(rss.load() / 1024));
		ImGui::Text("free RAM in MB: %llu, GB: %llu", (unsigned long long)freeRam.load(), (unsigned long long)(freeRam.load()/1024));
		ImGui::Text("frustum debug on: %d", debug.showFrustum);

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
	// Wake up all possible waits in worker thread
	realMeshesQueueCV.notify_all();
	worker1.join();
	chunks.clear();
	meshes.clear();
	glDeleteVertexArrays(1, &foliageVAO);
	glDeleteBuffers(1, &foliageVBO);

	// Clean up framebuffers before destroying OpenGL context
	horizontalBlurBuffer.cleanup();
	verticalBlurBuffer.cleanup();
	plainTerrainBuffer.cleanup();
	darkHorizontalBlurBuffer.cleanup();
	darkVerticalBlurBuffer.cleanup();

	shaderProgramBlocks.Delete();
	foliageShader.Delete();
	text1Shader.Delete();
	skyboxShader.Delete();
	lightObjectShader.Delete();
	lightOutlineShader.Delete();
	debugShader.Delete();
	horizontalBlurShader.Delete();
	verticalBlurShader.Delete();
	glfwTerminate();
	return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height){
	if (width == 0 || height == 0) return;
	window_width = width;
	window_height = height;
	projection = glm::perspective(glm::radians(camera.Zoom), float(window_width) / float(window_height), 0.1f, 20000.0f);
	horizontalBlurBuffer.screenUpdate(window_width, window_height);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
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
		process_button(BUTTON_LEFT, 263);
		process_button(BUTTON_RIGHT, 262);
		process_button(BUTTON_UP, 265);
		process_button(BUTTON_DOWN, 264);
		process_button(BUTTON_MINUS, 45);
		process_button(BUTTON_PLUS, 61);
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
		projection = glm::perspective(glm::radians(camera.Zoom), float(window_width) / float(window_height), 0.1f, 20000.0f);
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

void chunker1() {
	while (active == false) {

	}
	while (true) {
		if (!worker1Finished.load()) {
			if (makeChunksOrder.load()) {
				timeBenchmark(0);
				generate = true;

				clearingChunks = true;
				/*chunks.clear();
				size_t numMeshes = meshes.size();
				for (size_t i = 0; i < numMeshes; i++) {
					std::shared_ptr<Mesh> mesh = meshes.get(i);
					if (mesh) mesh->clearAndShrink();
					mesh.reset();
				}
				meshes.clear();
				chunks.shrink_to_fit();
				meshes.shrink_to_fit();
				trim_heap();*/
				clearingChunks = false;

				struct Voxel {
    				int x, y, z;
    				int distSq;
				};

				std::vector<Voxel> voxels;
				int radius = totalChunks;
    			int rSq = radius * radius;

				voxels.reserve(static_cast<size_t>(4.2 * radius * radius * radius));

				for (int x = -radius; x <= radius; ++x) {
        			int x2 = x * x;
        			for (int y = -radius; y <= radius; ++y) {
        			    int y2 = y * y;
        			    if (x2 + y2 > rSq) continue; 
					
        			    for (int z = -radius; z <= radius; ++z) {
        			        int z2 = z * z;
        			        int dSq = x2 + y2 * 4 + z2;
        			        if (dSq <= rSq) {
        			            voxels.push_back({x, y, z, dSq});
        			        }
        			    }
        			}
    			}

    			std::sort(voxels.begin(), voxels.end(), [](const Voxel& a, const Voxel& b) {
    			    return a.distSq < b.distSq;
    			});
			
    			for (const auto& v : voxels) {
                    if (joinableThreads[0] == false) break;
                    float chunkX = generatePos.x + v.x * 32.0f;
                    float chunkY = generatePos.y + (v.y - 1) * 32.0f;
                    float chunkZ = generatePos.z + v.z * 32.0f;


                    if(chunksSearch.find(glm::vec3(chunkX, chunkY, chunkZ)) == chunksSearch.end()){
                        auto newChunk = std::make_shared<Chunk>(chunkX, chunkY, chunkZ);
                        chunks.push_back(newChunk);
                        chunksSearch[glm::vec3(chunkX, chunkY, chunkZ)] = chunks.size() - 1;
                    }


                    unsigned int chunkIndex = chunksSearch.at(glm::vec3(chunkX, chunkY, chunkZ));
                    std::shared_ptr<Chunk> chunkPtr = chunks.get(chunkIndex);
                    if(!chunkPtr->empty && !chunkPtr->solid){
                        if(!chunkPtr->usedMesh){
                            auto newMesh = std::make_shared<Mesh>();
                            newMesh->chunksIndex = chunkIndex;
                            meshes.push_back(newMesh);
                            unsigned int meshIndex = static_cast<unsigned int>(meshes.size() - 1);
                            newMesh->fillChunk(chunkX, chunkY, chunkZ);
                            {
                                std::lock_guard<std::mutex> lock(realMeshesQueueMutex);
                                realMeshesQueue.push(meshIndex);
                            }
                            realMeshesQueueCV.notify_one();
                            completedChunks++;
                            newMesh.reset();
                            chunkPtr->usedMesh = true;
                            chunkPtr->meshIndex = meshIndex;
                        }else{
                            chunkPtr->distanceI = chunkPtr->neighborDistanceI(chunkX, chunkY, chunkZ);
                            std::shared_ptr<Mesh> mesh = meshes.get(chunkPtr->meshIndex);
                            
                        }
                    }

                    get_free_ram();
                    if(freeRam < 150){ //EMERGENCY ACTION!!! RAM APPROACHING SYSTEM CRASH PROTECTION
                        std::cerr << "\nRAM APPROACHING 100%, EMERGENCY CLEANUP STARTED\n";
                        get_used_ram();
                        size_t minEscape = 0;
                        rss.load() < 350 ? minEscape = 150 + rss.load() / 2 : minEscape = 350;
						std::cout << "minEscape: " << minEscape << std::endl;
                        while(freeRam < minEscape){
                            if (joinableThreads[0] == false) break;
                            if(chunks.size() >= 1){
								chunks.remove(0);
							}else{
								break;
							}
                            trim_heap();
                            get_free_ram();
                        }
                        chunks.shrink_to_fit();
                        trim_heap();
                        {
                            std::lock_guard<std::mutex> lk(chunksSearchMutex);
                            chunksSearch.clear();
                        }
                        size_t remaining = chunks.size();
                        for(size_t i = 0; i < remaining; i++){
                            std::shared_ptr<Chunk> c = chunks.get(i);
                            if(c) chunksSearch[glm::vec3(c->X, c->Y, c->Z)] = static_cast<unsigned int>(i);
                        }
                        std::cerr << "Emergency cleanup done\n";
                    }
            	}

				makeChunksOrder = false;
				worker1Finished = true;
				generate = false;
				timeBenchmark(1);
				trim_heap();
			}

		}
		if (joinableThreads[0] == false) break;
	}
	std::cout << "\nWORKER1 THREAD ENDED";
}

void setSpawn(float x, float z) {
	x = round(int(x) / 10) * 10.0f;
	z = round(int(z) / 10) * 10.0f;
	std::vector<float> meshHeights;
	size_t numChunks = chunks.size();
	if (numChunks > 0) {
		for (size_t i = 0; i < numChunks; i++) {
			std::shared_ptr<Chunk> chunk = chunks.get(i);
			std::shared_ptr<Mesh> mesh = meshes.get(i);
			if (chunk && mesh) {
				std::lock_guard<std::mutex> chunkLock(chunk->chunkMtx);
				std::lock_guard<std::mutex> meshLock(mesh->meshMtx);
				if (chunk->X == x && chunk->Z == z && mesh->vertices.size() > 0) meshHeights.push_back(chunk->Y);
			}
			mesh.reset();
		}
		if (!meshHeights.empty()) {
			auto autoMax = std::max_element(meshHeights.begin(), meshHeights.end());
			int max = *autoMax;
		}
		Chunk temp;
		camera.Position.y = temp.calcNoiseAbsolute(camera.Position.x, camera.Position.z) + 15;
	}
}

//in mb
void get_free_ram(){
#ifdef _WIN32
    MEMORYSTATUSEX memStatus;
    memStatus.dwLength = sizeof(memStatus);
    if (GlobalMemoryStatusEx(&memStatus))
        freeRam = static_cast<size_t>(memStatus.ullAvailPhys / (1024 * 1024));
#else
    struct sysinfo si;
    if (sysinfo(&si) == 0) freeRam = (si.freeram * si.mem_unit) / (1024 * 1024);
#endif
}


//in mb
void get_used_ram() {
#ifdef _WIN32
    PROCESS_MEMORY_COUNTERS_EX pmc;
    pmc.cb = sizeof(pmc);
    if (GetProcessMemoryInfo(GetCurrentProcess(),
            reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&pmc), sizeof(pmc)))
        rss.store(pmc.WorkingSetSize / (1024 * 1024));
#else
    std::ifstream stat_stream("/proc/self/statm", std::ios_base::in);
    if (!stat_stream) return;


    size_t total_pages, resident_pages;
    if (stat_stream >> total_pages >> resident_pages) {
        long page_size = sysconf(_SC_PAGESIZE);
        size_t current_rss_mb = (resident_pages * page_size) / (1024 * 1024);
       
        rss.store(current_rss_mb);
    }
#endif
}


void window_focus_callback(GLFWwindow* window, int focused) {
	for (int i = 0; i < BUTTON_COUNT; i++) {
        input.buttons[i].is_down = false;
        input.buttons[i].is_changed = false;
    }
	bool leftButtonPressed = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
    bool rightButtonPressed = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
    input.buttons[MOUSE_LEFT].is_down = leftButtonPressed;
    input.buttons[MOUSE_RIGHT].is_down = rightButtonPressed;
    if (focused) {
		glfwMakeContextCurrent(window);
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        lastX = static_cast<float>(xpos); 
        lastY = static_cast<float>(ypos);
        firstMouse = false;
    }else{
		firstMouse = true;
	}
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods){

#define process_button(b, keyCode)\
case keyCode: {\
input.buttons[b].is_changed = (bool(action) != input.buttons[b].is_down);\
input.buttons[b].is_down = bool(action);\
} break;\

	switch (button){
		process_button(MOUSE_LEFT, 0);
		process_button(MOUSE_RIGHT, 1);
		process_button(MOUSE_SCROLL_WHEEL, 2);
		process_button(MOUSE_LEFT_BACK, 3);
		process_button(MOUSE_LEFT_FRONT, 4);
	default:
		break;
	}
}