#include "classes.h"
#include "tileHandling.h"
#include "cameraClass.h"
#include "glad.h"
#include <iostream>

// Global variable definitions
Input input;
Debug debug;
short distanceIncriment[] = {
	1, 2, 2, 4, 4, 8, 10, 10, 20, 20, 20, 20, 20, 20, 40
};
glm::vec3 generatePos = glm::vec3(0.0f, 0.0f, 0.0f);
std::vector<std::string> ramChunksStrings;
std::vector<objectData> ramChunks;
float SPEED = 8.0f;
float SENSITIVITY = 0.1f;

Shader::Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath) {
    std::string vertexCode;
    std::string fragmentCode;
    std::string geometryCode;
    std::ifstream vShaderFile;
    std::ifstream fShaderFile;
    std::ifstream gShaderFile;
    vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    gShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try {
        vShaderFile.open(vertexPath);
        fShaderFile.open(fragmentPath);
        std::stringstream vShaderStream, fShaderStream;
        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();
        vShaderFile.close();
        fShaderFile.close();
        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();
        if (geometryPath != nullptr) {
            gShaderFile.open(geometryPath);
            std::stringstream gShaderStream;
            gShaderStream << gShaderFile.rdbuf();
            gShaderFile.close();
            geometryCode = gShaderStream.str();
        }
    } catch (std::ifstream::failure& e) {
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << std::endl;
    }
    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();
    unsigned int vertex, fragment;
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);
    checkCompileErrors(vertex, "VERTEX");
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);
    checkCompileErrors(fragment, "FRAGMENT");
    unsigned int geometry;
    if (geometryPath != nullptr) {
        const char* gShaderCode = geometryCode.c_str();
        geometry = glCreateShader(GL_GEOMETRY_SHADER);
        glShaderSource(geometry, 1, &gShaderCode, NULL);
        glCompileShader(geometry);
        checkCompileErrors(geometry, "GEOMETRY");
    }
    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    if (geometryPath != nullptr)
        glAttachShader(ID, geometry);
    glLinkProgram(ID);
    checkCompileErrors(ID, "PROGRAM");
    glDeleteShader(vertex);
    glDeleteShader(fragment);
    if (geometryPath != nullptr)
        glDeleteShader(geometry);
}
void Shader::use(){ 
    glUseProgram(ID); 
}
void Shader::Delete(){ 
    glDeleteProgram(ID); 
}
void Shader::setBool(const std::string& name, bool value) const { glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value); }
void Shader::setInt(const std::string& name, int value) const { glUniform1i(glGetUniformLocation(ID, name.c_str()), value); }
void Shader::setFloat(const std::string& name, float value) const { glUniform1f(glGetUniformLocation(ID, name.c_str()), value); }
void Shader::setVec2(const std::string& name, const glm::vec2& value) const { glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]); }
void Shader::setVec2(const std::string& name, float x, float y) const { glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y); }
void Shader::setVec3(const std::string& name, const glm::vec3& value) const { glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]); }
void Shader::setVec3(const std::string& name, float x, float y, float z) const { glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z); }
void Shader::setVec4(const std::string& name, const glm::vec4& value) const { glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]); }
void Shader::setVec4(const std::string& name, float x, float y, float z, float w) { glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w); }
void Shader::setMat2(const std::string& name, const glm::mat2& mat) const { glUniformMatrix2fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]); }
void Shader::setMat3(const std::string& name, const glm::mat3& mat) const { glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]); }
void Shader::setMat4(const std::string& name, const glm::mat4& mat) const { glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]); }

void Shader::checkCompileErrors(GLuint shader, std::string type) {
    GLint success;
    GLchar infoLog[1024];
    if (type != "PROGRAM") {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    } else {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
}

Texture::Texture(const char* texturePaths[], short int textureNumber, GLint internalformat, bool atlas, int size) {
    glGenTextures(1, &ID);
    glActiveTexture(GL_TEXTURE0 + textureNumber);
    glBindTexture(GL_TEXTURE_2D, ID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    if (atlas) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, size - 1);
        for (int i = 0; i < size; i++) {
            int width, height, nrChannels;
            data = stbi_load(texturePaths[i], &width, &height, &nrChannels, 0);
            if (data) {
                glTexImage2D(GL_TEXTURE_2D, i, internalformat, width, height, 0, internalformat, GL_UNSIGNED_BYTE, data);
            }
            else {
                std::cout << "Failed to load texture" << std::endl;
            }
            stbi_image_free(data);
        }
    }
    else {
        int width, height, nrChannels;
        data = stbi_load(texturePaths[0], &width, &height, &nrChannels, 0);
        if (data) {
            glTexImage2D(GL_TEXTURE_2D, 0, internalformat, width, height, 0, internalformat, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        else {
            std::cout << "Failed to load texture" << std::endl;
        }
        stbi_image_free(data);
    }
}

Chunk::Chunk() { initialNoiseSet(); }

Chunk::Chunk(float X, float Y, float Z) {
    initialNoiseSet();
    create(X, Y, Z);
}

void Chunk::initialNoiseSet() {
    this->noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2S);
    this->noise.SetSeed(0);
    this->noise.SetFrequency(0.002f);
    this->noise.SetFractalType(FastNoiseLite::FractalType_FBm);
    this->noise.SetFractalOctaves(5);
    this->noise.SetFractalLacunarity(2.0f);
    this->noise.SetFractalGain(0.5f);
    this->noise.SetFractalWeightedStrength(0.0f);
    largeNoise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    largeNoise.SetSeed(0);
    largeNoise.SetFrequency(0.002f);
}

float Chunk::calcNoise(float x, float z) {
    float worldX = x + this->X;
    float worldZ = z + this->Z;     
    float currentNoise = this->noise.GetNoise(worldX, worldZ);
    float plusNoise = largeNoise.GetNoise(worldX, worldZ);
    plusNoise *= 200.0f;
    currentNoise *= 10.0f;
    currentNoise = currentNoise * currentNoise * currentNoise;
    currentNoise -= (std::sin(worldX) * std::cos(worldZ)) * (currentNoise / 70);
    currentNoise += plusNoise;
    return currentNoise;
}

float Chunk::calcNoiseAbsolute(float x, float z) {
    float currentNoise = noise.GetNoise(x, z);
    float plusNoise = largeNoise.GetNoise(x, z);
    plusNoise *= 200.0f;
    currentNoise *= 10.0f;
    currentNoise = currentNoise * currentNoise * currentNoise;
    currentNoise -= (std::sin(x) * std::cos(z)) * (currentNoise / 70);
    currentNoise += plusNoise;
    return currentNoise;
}

void Chunk::create(float X, float Y, float Z) {
        this->X = X;
        this->Y = Y;
        this->Z = Z;
        this->solid = true;
        this->empty = true;
        //35 chunks: 15.783    PB: 15.783
        //12 chunks: 6.905     PB: 6.905
        distanceI = neighborDistanceI(X, Y, Z);
        
        for (int z = 0; z < widths; z++) {
            for (int x = 0; x < widths; x++) {
                float currentNoise = calcNoise(x, z);
                int surfaceY = std::max(-1, std::min(int(widths), (int)std::floor(currentNoise - Y)));
                heightMap[x][z] = (char)surfaceY;
                if (surfaceY >= 0) empty = false;
                if (surfaceY < widths) solid = false;
            }
        }
}

short Chunk::neighborDistanceI(float chunkX, float chunkY, float chunkZ) {
    short neighborDistance = trunc(sqrt(((chunkX) - generatePos.x) * ((chunkX) - generatePos.x) + ((chunkY) - generatePos.y) * ((chunkY) - generatePos.y) + ((chunkZ) - generatePos.z) * ((chunkZ) - generatePos.z))) / 200;
    if (neighborDistance > 14) neighborDistance = 14;
    return distanceIncriment[neighborDistance];
}

Chunk::~Chunk(){}

void Chunk::writeChunk(){
    /*std::fstream chunkFile;
    chunkFile.open("../resources/data/storedChunks.wrld", std::ios::app);
    if (chunkFile.is_open()) {
        std::string line = "";
        line += ("#" + std::to_string(int(X)) + "|" + std::to_string(int(Y)) + "|" + std::to_string(int(Z)) + "|");
        int amount = 0;
        short currentObject = 0;
        for (int i = 0; i < 64000; i++) {
            if (this->objects[i] == currentObject) {
                amount++;
            }
            else {
                if (amount > 0)  line += (std::to_string(currentObject) + ">" + std::to_string(amount) + "|");
                currentObject = this->objects[i];
                amount = 1;
            }
        }
        line += std::to_string(currentObject) + ">" + std::to_string(amount) + "|#" + std::to_string(int(empty)) + std::to_string(int(solid));
        
        chunkFile << line << '\n';
        ramChunksStrings.push_back(line);
        chunkFile.close();
    }*/
}

void Chunk::readChunkString(unsigned int lineNumber) {
    /*unsigned int place = 0;
    //test...
    std::string line = ramChunksStrings[lineNumber];
    for (int i = 0; i < 3; i++) {
        while (line[place] != '|') {
            place++;
        }
        place++;
    }
    unsigned short type = line[place] - '0';
    place += 2;
    unsigned int previ = 0;
    empty = bool(line[line.length() - 2] - '0');
    solid = bool(line[line.length() - 1] - '0');
    if (empty) {
        std::fill(std::begin(objects), std::end(objects), 0);
    } else {
        while (line[place] != '#') {
            std::string snum = "";
            while (line[place] != '|') {
                snum += line[place++];
            }
            for (int i = 0; i < std::stoi(snum); i++) {
                this->objects[previ++] = type;
            }
            place++;
            if (line[place] != '#') {
                type = line[place] - '0';
                place += 2;
            }
            else {
                break;
            }
        }
    }*/
}

unsigned int Chunk::search(float X, float Y, float Z) {
    int x = 0, y = 0, z = 0;
    for(int j = 0; j < ramChunksStrings.size(); j++){
        std::string line = ramChunksStrings[j];
        if (line[0] == '#') {
            unsigned int i = 1;
            std::string sx = "";
            while (line[i] != '|') {
                sx += line[i++];
            }
            i++;
            x = static_cast<int>(std::stoi(sx));
            std::string sy = "";
            while (line[i] != '|') {
                sy += line[i++];
            }
            i++;
            y = static_cast<int>(std::stoi(sy));
            std::string sz = "";
            while (line[i] != '|') {
                sz += line[i++];
            }
            i++;
            z = static_cast<int>(std::stoi(sz));
            if (x == X && y == Y && z == Z) {
                return j;
            }
        }
    }
    return 0;
}

void Chunk::clearAndShrink() {
	/*std::lock_guard<std::mutex> lock(chunkMtx);
	objects.clear();
	objects.shrink_to_fit();*/
}

Mesh::Mesh(){}

void Mesh::init() {
    glGenVertexArrays(1, &this->VAO);
    glGenBuffers(1, &this->VBO);
}

void Mesh::cleanup() {
    if (deleted) return;

    if (VAO != 0) {
        glDeleteVertexArrays(1, &this->VAO);
        VAO = 0;
    }
    if (VBO != 0) {
        glDeleteBuffers(1, &this->VBO);
        VBO = 0;
    }
    deleted = true;
}

Mesh::~Mesh() {
    cleanup();
}

void Mesh::updateBuffers() {
    glBindVertexArray(this->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
    glBufferData(GL_ARRAY_BUFFER, this->vertices.size() * sizeof(unsigned int), this->vertices.data(), GL_DYNAMIC_DRAW);
    glVertexAttribIPointer(0, 1, GL_UNSIGNED_INT, sizeof(unsigned int), (void*)0);
    glEnableVertexAttribArray(0);
}

void Mesh::listItems(bool which) {
    if (!which) {
        std::cout << "***********VERTICES LIST***********" << '\n';
        std::cout << "length: " << this->vertices.size() << '\n';
        for (size_t i = 0; i < this->vertices.size(); i++) {
            std::cout << this->vertices[i] << " (Binary: " << std::bitset<32>(this->vertices[i]) << ")" << '\n';
        }
    }
}

void Mesh::fillChunk(float chunkX, float chunkY, float chunkZ) {
    this->vertices.clear();
    std::shared_ptr<Chunk> chunkPtr = chunks.get(chunksIndex);
    if (!chunkPtr) return;
    if(chunksSearch.find(glm::vec3(chunkX - 40, chunkY, chunkZ)) == chunksSearch.end()){
		auto newChunk = std::make_shared<Chunk>(chunkX - 40, chunkY, chunkZ);
		chunks.push_back(newChunk);
		chunksSearch[glm::vec3(chunkX - 40, chunkY, chunkZ)] = chunks.size() - 1;
        chunksLeftIndex = chunks.size() - 1;
	}else{
        chunksLeftIndex = chunksSearch.at(glm::vec3(chunkX - 40, chunkY, chunkZ));
    }
    std::shared_ptr<Chunk> chunkLeftPtr = chunks.get(chunksLeftIndex);
    if (!chunkLeftPtr) return;

    if(chunksSearch.find(glm::vec3(chunkX + 40, chunkY, chunkZ)) == chunksSearch.end()){
		auto newChunk = std::make_shared<Chunk>(chunkX + 40, chunkY, chunkZ);
		chunks.push_back(newChunk);
		chunksSearch[glm::vec3(chunkX + 40, chunkY, chunkZ)] = chunks.size() - 1;
        chunksRightIndex = chunks.size() - 1;
	}else{
        chunksRightIndex = chunksSearch.at(glm::vec3(chunkX + 40, chunkY, chunkZ));
    }
    std::shared_ptr<Chunk> chunkRightPtr = chunks.get(chunksRightIndex);
    if (!chunkRightPtr) return;

    if(chunksSearch.find(glm::vec3(chunkX, chunkY, chunkZ - 40)) == chunksSearch.end()){
		auto newChunk = std::make_shared<Chunk>(chunkX, chunkY, chunkZ - 40);
		chunks.push_back(newChunk);
		chunksSearch[glm::vec3(chunkX, chunkY, chunkZ - 40)] = chunks.size() - 1;
        chunksBackIndex = chunks.size() - 1;
	}else{
        chunksBackIndex = chunksSearch.at(glm::vec3(chunkX, chunkY, chunkZ - 40));
    }
    std::shared_ptr<Chunk> chunkBackPtr = chunks.get(chunksBackIndex);
    if (!chunkBackPtr) return;

     if(chunksSearch.find(glm::vec3(chunkX, chunkY, chunkZ + 40)) == chunksSearch.end()){
		auto newChunk = std::make_shared<Chunk>(chunkX, chunkY, chunkZ + 40);
		chunks.push_back(newChunk);
		chunksSearch[glm::vec3(chunkX, chunkY, chunkZ + 40)] = chunks.size() - 1;
        chunksFrontIndex = chunks.size() - 1;
	}else{
        chunksFrontIndex = chunksSearch.at(glm::vec3(chunkX, chunkY, chunkZ + 40));
    }
    std::shared_ptr<Chunk> chunkFrontPtr = chunks.get(chunksFrontIndex);
    if (!chunkFrontPtr) return;
    
    std::lock_guard<std::mutex> chunkLock(chunkPtr->chunkMtx);
    Chunk& chunk = *chunkPtr;
    std::lock_guard<std::mutex> chunkLock2(chunkLeftPtr->chunkMtx);
    Chunk& leftChunk = *chunkLeftPtr;
    std::lock_guard<std::mutex> chunkLock3(chunkRightPtr->chunkMtx);
    Chunk& rightChunk = *chunkRightPtr;
    std::lock_guard<std::mutex> chunkLock4(chunkBackPtr->chunkMtx);
    Chunk& backChunk = *chunkBackPtr;
    std::lock_guard<std::mutex> chunkLock5(chunkFrontPtr->chunkMtx);
    Chunk& frontChunk = *chunkFrontPtr;

    this->X = chunk.X;
    this->Y = chunk.Y;
    this->Z = chunk.Z;
    this->distanceI = chunk.distanceI;
    std::vector<unsigned int> verticesTemp;
    for (int z = 0; z < chunk.widths; z += this->distanceI) {
        for (int x = 0; x < chunk.widths; x += this->distanceI) {
            int y = (int)chunk.heightMap[x][z];
            int yNegX = x - distanceI < 0 ? (int)leftChunk.heightMap[40 - distanceI][z] : (int)chunk.heightMap[x - distanceI][z];
            int yPosX = x + distanceI > 39 ? (int)rightChunk.heightMap[x + distanceI - chunk.widths][z] : (int)chunk.heightMap[x + distanceI][z];
            int yNegZ = z - distanceI < 0 ? (int)backChunk.heightMap[x][40 - distanceI] : (int)chunk.heightMap[x][z - distanceI];
            int yPosZ = z + distanceI > 39 ? (int)frontChunk.heightMap[x][z + distanceI - chunk.widths] : (int)chunk.heightMap[x][z + distanceI];
            if(y >= 0){
            unsigned int current = 0;

#define addVertices(exposed)\
for (unsigned int i = (6 * exposed); i < (6 * exposed) + 6; i++) {\
    unsigned int tempVertice = naturalTiles[current].data[i];\
    tempVertice |= static_cast<unsigned int>(x + chunk.widths * (z + chunk.widths * y)) << 16;\
    verticesTemp.push_back(tempVertice);\
}\

            // y-min (face 0)
            // y-max (face 1)
            
            if(y != 40){
                current = 2; //grass
                addVertices(1);
                if(y > yNegX) addVertices(2);
                if(y > yPosX) addVertices(3);
                if(y > yNegZ) addVertices(4);
                if(y > yPosZ) addVertices(5);
                y-=distanceI;
                if(y < 0) continue;
                current = 3; //dirt
                if(y > yNegX) addVertices(2);
                if(y > yPosX) addVertices(3);
                if(y > yNegZ) addVertices(4);
                if(y > yPosZ) addVertices(5);
                y-=distanceI;
                if(y < 0) continue;
                current = 1; //stone
                while(y >= 0){
                    if(y > yNegX) addVertices(2);
                    if(y > yPosX) addVertices(3);
                    if(y > yNegZ) addVertices(4);
                    if(y > yPosZ) addVertices(5);
                    y-=distanceI;
                }
            }else{
                y--;
                int currentHeight = std::floor(chunk.calcNoise(x, z));
                if(currentHeight - (Y + chunk.widths) == 0){
                    current = 3; //dirt
                    if(y > yNegX) addVertices(2);
                    if(y > yPosX) addVertices(3);
                    if(y > yNegZ) addVertices(4);
                    if(y > yPosZ) addVertices(5);
                    y-=distanceI;
                }
                current = 1; //stone
                while(y >= 0){
                    if(y > yNegX) addVertices(2);
                    if(y > yPosX) addVertices(3);
                    if(y > yNegZ) addVertices(4);
                    if(y > yPosZ) addVertices(5);
                    y-=distanceI;
                }
            }    
            } //y >= 0
        }
    }
    this->vertices = verticesTemp;
}

void Mesh::clearAndShrink() {
	std::lock_guard<std::mutex> lock(meshMtx);
	vertices.clear();
	vertices.shrink_to_fit();
}

void frameBuffer::screenUpdate(int newWidth, int newHeight) {
    if (newWidth <= 0 || newHeight <= 0) return;
    if ((unsigned)newWidth == width && (unsigned)newHeight == height) return;
    width = static_cast<unsigned int>(newWidth);
    height = static_cast<unsigned int>(newHeight);
    // Update color texture storage
    glBindTexture(GL_TEXTURE_2D, colorAttachment);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    // Update renderbuffer storage
    glBindRenderbuffer(GL_RENDERBUFFER, depthAttachment);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    // Keep texture bound as caller needs it; no framebuffer rebind required here
}

void frameBuffer::build(unsigned int width, unsigned int height) {
    glGenFramebuffers(1, &frameBufferPrivate);
    glBindFramebuffer(GL_FRAMEBUFFER, frameBufferPrivate);
    glGenTextures(1, &colorAttachment);
    glBindTexture(GL_TEXTURE_2D, colorAttachment);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorAttachment, 0);
    glGenRenderbuffers(1, &depthAttachment);
    glBindRenderbuffer(GL_RENDERBUFFER, depthAttachment);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthAttachment);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    this->width = width;
    this->height = height;
}

void frameBuffer::cleanup() {
    if (deleted) return;

    if (frameBufferPrivate != 0) {
        glDeleteFramebuffers(1, &frameBufferPrivate);
        frameBufferPrivate = 0;
    }
    if (colorAttachment != 0) {
        glDeleteTextures(1, &colorAttachment);
        colorAttachment = 0;
    }
    if (depthAttachment != 0) {
        glDeleteRenderbuffers(1, &depthAttachment);
        depthAttachment = 0;
    }
    deleted = true;
}

frameBuffer::~frameBuffer() {
    cleanup();
}

void frameBuffer::drawTo() {
    glBindFramebuffer(GL_FRAMEBUFFER, frameBufferPrivate);
    glViewport(0, 0, width > 0 ? width : 1, height > 0 ? height : 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void frameBuffer::readFrom() {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, colorAttachment);
}
