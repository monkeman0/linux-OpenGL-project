#ifndef SHADER_H
#define SHADER_H

#include "glad.h"
#include "stb_image.h"
#include "cameraClass.h"
#include <fstream>
#include <sstream>
#include <glm/gtc/type_ptr.hpp>
#include <FastNoise/FastNoise.h>
#include "inputHandler.cpp"
#include <bitset>
#include <map>
#include <mutex>
#include <memory>
#include "chunksSearch.h"

class Chunk;
template <typename T>
class ThreadSafeVector;
extern int totalChunksGenerated;
extern float bytesFromMeshes;
extern ThreadSafeVector<Chunk> chunks;
extern std::atomic<bool> clearingChunks;
extern Input input;
extern Debug debug;
extern short distanceIncriment[];
extern glm::vec3 generatePos;

class Shader {
public:
    unsigned int ID;
    Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath = nullptr);
    void use();
    void Delete();
    void setBool(const std::string& name, bool value) const;
    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const;
    void setVec2(const std::string& name, const glm::vec2& value) const;
    void setVec2(const std::string& name, float x, float y) const;
    void setVec3(const std::string& name, const glm::vec3& value) const;
    void setVec3(const std::string& name, float x, float y, float z) const;
    void setVec4(const std::string& name, const glm::vec4& value) const;
    void setVec4(const std::string& name, float x, float y, float z, float w);
    void setMat2(const std::string& name, const glm::mat2& mat) const;
    void setMat3(const std::string& name, const glm::mat3& mat) const;
    void setMat4(const std::string& name, const glm::mat4& mat) const;
private:
    void checkCompileErrors(GLuint shader, std::string type);
};

class Texture {
public:
    unsigned int ID;
    unsigned char* data;
    Texture(const char* texturePaths[], short int textureNumber, GLint internalformat, bool atlas, int size);
};

extern std::vector<std::string> ramChunksStrings;

class Mesh {
public:
    float X = 0.0f;
    float Y = 0.0f;
    float Z = 0.0f;
    std::vector<unsigned int> vertices;
    unsigned int VAO = 0, VBO = 0;
    short distanceI = 1;
    unsigned int chunksIndex = 0;
    unsigned int chunksLeftIndex = 0;
    unsigned int chunksRightIndex = 0;
    unsigned int chunksBackIndex = 0;
    unsigned int chunksFrontIndex = 0;
    bool deleted = false;
    mutable std::mutex meshMtx;
    bool readable = false;
    bool LODborder = false;

    Mesh();
    void init();
    void cleanup();
    ~Mesh();
    void updateBuffers();
    void listItems(bool which);
    unsigned int size(bool type);
    void fillChunk(float chunkX, float chunkY, float chunkZ);
	void clearAndShrink();

private:
};

class Chunk {
public:
    static const short widths = 32;
    char heightMap[32][32] = { '0' };
    bool empty = true;
    bool solid = true;
    float X = 0;
    float Y = 0;
    float Z = 0;
    short distanceI = 1;
    mutable std::mutex chunkMtx;
    bool usedMesh = false;
    unsigned int meshIndex = 0;

    Chunk();
    Chunk(float X, float Y, float Z);
    float calcNoiseAbsolute(float x, float z);
    void create(float X, float Y, float Z);
    short neighborDistanceI(float chunkX, float chunkY, float chunkZ);
    void clearAndShrink();
    static void noiseInit();
    ~Chunk();
private:

    void writeChunk();
    void readChunkString(unsigned int lineNumber);
    unsigned int search(float X, float Y, float Z);
};


class frameBuffer {
public:
    bool deleted = false;
    void screenUpdate(int newWidth, int newHeight);
    void build(unsigned int width, unsigned int height);
    void cleanup();
    ~frameBuffer();
    void drawTo();
    void readFrom();
private:
    unsigned int frameBufferPrivate, colorAttachment, depthAttachment, width, height;
};

template <typename T>
class ThreadSafeVector {
private:
    std::vector<std::shared_ptr<T>> data;
    mutable std::mutex vectorMtx;

public:
    
    void push_back(const std::shared_ptr<T>& value){
        std::lock_guard<std::mutex> lock(vectorMtx);
        data.push_back(value);
    }

    std::shared_ptr<T> get(size_t index){
        std::lock_guard<std::mutex> lock(vectorMtx);
        if (index < data.size()) return data[index];
        return nullptr;
    }

    size_t size() const{
        std::lock_guard<std::mutex> lock(vectorMtx);
        return data.size();
    }

    void remove(size_t index){
        std::lock_guard<std::mutex> lock(vectorMtx);
        data.erase(data.begin() + index);
    }

    void clear(){
        std::lock_guard<std::mutex> lock(vectorMtx);
        data.clear();
    }
	void shrink_to_fit(){
		std::lock_guard<std::mutex> lock(vectorMtx);
		data.shrink_to_fit();
	}

    size_t push_back_and_index(const std::shared_ptr<T>& value) {
        std::lock_guard<std::mutex> lock(vectorMtx);
        data.push_back(value);
        return data.size() - 1;
    }
};

#endif
