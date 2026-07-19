#include "classes.h"
#include "tileHandling.h"
#include "cameraClass.h"
#include "glad.h"
#include <iostream>

// Global variable definitions
Input input;
Debug debug;
short distanceIncriment[] = {
	1, 2, 2, 4, 4, 8, 10, 10, 20, 20, 20, 20, 20, 20, 32
};
glm::vec3 generatePos = glm::vec3(0.0f, 0.0f, 0.0f);
std::vector<std::string> ramChunksStrings;
float SPEED = 8.0f;
float SENSITIVITY = 0.1f;
extern std::mutex chunksSearchMutex;
auto finalNode = FastNoise::New<FastNoise::DomainScale>();

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

Chunk::Chunk() {}

void Chunk::noiseInit(){
    auto perlin = FastNoise::New<FastNoise::Perlin>();
    perlin->SetOutputMin(-1.f);
    perlin->SetOutputMax(1.f);
    perlin->SetScale(3500.f);

    auto scale = FastNoise::New<FastNoise::DomainScale>();
    scale->SetSource(perlin);
    scale->SetScaling(5.06f);/*

    auto fractalFBM = FastNoise::New<FastNoise::FractalFBm>();
    fractalFBM->SetSource(scale);
    fractalFBM->SetGain(0.6f);
    fractalFBM->SetLacunarity(2.f);
    fractalFBM->SetOctaveCount(5);
    fractalFBM->SetWeightedStrength(1.f);

    auto domainWarp = FastNoise::New<FastNoise::DomainWarpSimplex>();
    domainWarp->SetSource(fractalFBM);
    domainWarp->SetWarpAmplitude(12.7f);
    domainWarp->SetScale(50.f);
    domainWarp->SetAmplitudeScaling<FastNoise::Dim::X>(1.1f);
    domainWarp->SetAmplitudeScaling<FastNoise::Dim::Y>(1.3f);

    auto add2 = FastNoise::New<FastNoise::Subtract>();
    add2->SetLHS(perlin);
    add2->SetRHS(domainWarp);*/

    finalNode->SetSource(scale);
}

Chunk::Chunk(float X, float Y, float Z) {
    create(X, Y, Z);
}

float Chunk::calcNoiseAbsolute(float x, float z) {
    return 0;
}

void Chunk::create(float X, float Y, float Z) {
    this->X = X;
    this->Y = Y;
    this->Z = Z;
    this->solid = true;
    this->empty = true;
    //35 chunks: 15.783    PB: 15.783
    //12 chunks: 6.905     PB: 6.905
    auto subChunk = chunksSearch.find(glm::vec3(X, Y - 32, Z));
    if(subChunk != chunksSearch.end()){
        unsigned int subIndex = subChunk->second;
        std::shared_ptr<Chunk> chunkPtr = chunks.get(subIndex);
        if (chunkPtr){
            if(chunkPtr->empty){
                this->solid = false;
                this->empty = true;
                for (int x = 0; x < 32; x++) {
                    for (int z = 0; z < 32; z++) {
                        heightMap[x][z] = -1; 
                    }
                }
                return;
            }
            
        }
    }
    auto upperChunk = chunksSearch.find(glm::vec3(X, Y + 32, Z));
    if(upperChunk != chunksSearch.end()){
        unsigned int upperIndex = upperChunk->second;
        std::shared_ptr<Chunk> chunkPtr = chunks.get(upperIndex);
        if (chunkPtr){
            if(chunkPtr->solid){
                this->solid = true;
                this->empty = false;
                for (int x = 0; x < 32; x++) {
                    for (int z = 0; z < 32; z++) {
                        heightMap[x][z] = 32; 
                    }
                }
                return;
            }
            
        }
    }
    distanceI = neighborDistanceI(X, Y, Z);
    float roundX = std::floor(X / 64.0f) * 64.0f;
    float roundZ = std::floor(Z / 64.0f) * 64.0f;

    auto [it, inserted] = noiseSearch.try_emplace(glm::vec2(roundX, roundZ));
    if (inserted) {
        finalNode->GenUniformGrid2D(it->second, roundX, roundZ, 9, 9, 8.f, 8.f, 1);
    }
    auto& noiseData = it->second;

    float localX_start = X - roundX;
    float localZ_start = Z - roundZ;

    for (int z = 0; z < widths; z++) {
        // Current global-to-regional coordinate for this specific vertex
        float currentRegionZ = localZ_start + z;

        // Find the bounding indices in the 8x8 noise grid (spacing is 8 units)
        int z0 = std::min(8, static_cast<int>(std::floor(currentRegionZ / 8.0f)));
        int z1 = std::min(8, z0 + 1);

        // Calculate interpolation weight [0.0, 1.0] between z0 and z1
        float tZ = (currentRegionZ / 8.0f) - z0;

        for (int x = 0; x < widths; x++) {
            float currentRegionX = localX_start + x;

            int x0 = std::min(8, static_cast<int>(std::floor(currentRegionX / 8.0f)));
            int x1 = std::min(8, x0 + 1);
            float tX = (currentRegionX / 8.0f) - x0;

            // Sample the 4 surrounding corners from the 8x8 noise data
            float n00 = noiseData[z0 * 9 + x0]; // Top-Left
            float n10 = noiseData[z0 * 9 + x1]; // Top-Right
            float n01 = noiseData[z1 * 9 + x0]; // Bottom-Left
            float n11 = noiseData[z1 * 9 + x1]; // Bottom-Right

            // Bilinear interpolation
            float noise0 = n00 + tX * (n10 - n00); // Interpolate top row
            float noise1 = n01 + tX * (n11 - n01); // Interpolate bottom row
            float interpolatedNoise = noise0 + tZ * (noise1 - noise0); // Blend rows together

            float ridge = interpolatedNoise + 0.1f;
            ridge -= 0.11f * std::clamp(ridge / 1.5f, -1.f, 1.f);

            float currentNoise = std::abs((ridge * ridge * ridge) * 250.f);
            int surfaceY = std::max(-1, std::min(int(widths), (int)std::floor(currentNoise - Y)));

            heightMap[x][z] = (char)surfaceY;

            if (surfaceY >= 0) empty = false;
            if (surfaceY < widths) solid = false;
        }
    }
}

short Chunk::neighborDistanceI(float chunkX, float chunkY, float chunkZ) {
    float dx = chunkX - generatePos.x;
    float dy = chunkY - generatePos.y;
    float dz = chunkZ - generatePos.z;
    float distSq = dx*dx + dy*dy + dz*dz;
    // Pre-compute thresholds as: (i * 200)^2 for i in 0..14
    static const float thresholdsSq[] = {
        0, 40000, 160000, 360000, 640000, 1000000,
        1440000, 1960000, 640000, 3240000, 4000000,
        4840000, 5760000, 6760000, 7840000
    };
    short idx = 0;
    while (idx < 14 && distSq >= thresholdsSq[idx + 1]) idx++;
    return distanceIncriment[idx];
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
    std::shared_ptr<Chunk> chunkPtr = chunks.get(chunksIndex);
    if (!chunkPtr) return;
    std::lock_guard<std::mutex> chunkLock(chunkPtr->chunkMtx);
    Chunk& chunk = *chunkPtr;
    chunk.usedMesh = false;
}

void Mesh::updateBuffers() {
    glBindVertexArray(this->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
    glBufferData(GL_ARRAY_BUFFER, this->pendingVertices.size() * sizeof(unsigned int), this->pendingVertices.data(), GL_DYNAMIC_DRAW);
    glVertexAttribIPointer(0, 1, GL_UNSIGNED_INT, sizeof(unsigned int), (void*)0);
    glEnableVertexAttribArray(0);
    vertices = std::move(pendingVertices);
    distanceI = pendingDistanceI;
    pendingVertices.clear();
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
    std::shared_ptr<Chunk> chunkPtr = chunks.get(chunksIndex);
    if (!chunkPtr) return;

    auto [it, inserted] = chunksSearch.try_emplace(glm::vec3(chunkX - 32, chunkY, chunkZ));
    if (inserted) {
        auto newChunk = std::make_shared<Chunk>(chunkX - 32, chunkY, chunkZ);
        chunksLeftIndex = chunks.push_back_and_index(newChunk);
        it->second = chunksLeftIndex;
    }else{
        chunksLeftIndex = it->second;
    }
    std::shared_ptr<Chunk> chunkLeftPtr = chunks.get(chunksLeftIndex);
    if (!chunkLeftPtr) return;

    auto [it2, inserted2] = chunksSearch.try_emplace(glm::vec3(chunkX + 32, chunkY, chunkZ));
    if (inserted2) {
        auto newChunk = std::make_shared<Chunk>(chunkX + 32, chunkY, chunkZ);
        chunksRightIndex = chunks.push_back_and_index(newChunk);
        it2->second = chunksRightIndex;
    }else{
        chunksRightIndex = it2->second;
    }
    std::shared_ptr<Chunk> chunkRightPtr = chunks.get(chunksRightIndex);
    if (!chunkRightPtr) return;

    auto [it3, inserted3] = chunksSearch.try_emplace(glm::vec3(chunkX, chunkY, chunkZ - 32));
    if (inserted3) {
        auto newChunk = std::make_shared<Chunk>(chunkX, chunkY, chunkZ - 32);
        chunksBackIndex = chunks.push_back_and_index(newChunk);
        it3->second = chunksBackIndex;
    }else{
        chunksBackIndex = it3->second;
    }
    std::shared_ptr<Chunk> chunkBackPtr = chunks.get(chunksBackIndex);
    if (!chunkBackPtr) return;

    auto [it4, inserted4] = chunksSearch.try_emplace(glm::vec3(chunkX, chunkY, chunkZ + 32));
    if (inserted4) {
        auto newChunk = std::make_shared<Chunk>(chunkX, chunkY, chunkZ + 32);
        chunksFrontIndex = chunks.push_back_and_index(newChunk);
        it4->second = chunksFrontIndex;
    }else{
        chunksFrontIndex = it4->second;
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

    float roundX = std::floor(X / 64.0f) * 64.0f;
    float roundZ = std::floor(Z / 64.0f) * 64.0f;
    auto noiseKey = noiseSearch.find(glm::vec2(roundX, roundZ));

    int localX = static_cast<int>(X - roundX);
    int localZ = static_cast<int>(Z - roundZ);

    std::vector<unsigned int> verticesTemp;
    for (int z = 0; z < chunk.widths; z += this->pendingDistanceI) {
        for (int x = 0; x < chunk.widths; x += this->pendingDistanceI) {
            int y = (int)chunk.heightMap[x][z];
            if (y >= 0) y = (y / pendingDistanceI) * pendingDistanceI;

            int yNegX = x - pendingDistanceI < 0 ? (int)leftChunk.heightMap[32 - pendingDistanceI][z] : (int)chunk.heightMap[x - pendingDistanceI][z];
            int yPosX = x + pendingDistanceI > 31 ? (int)rightChunk.heightMap[x + pendingDistanceI - chunk.widths][z] : (int)chunk.heightMap[x + pendingDistanceI][z];
            int yNegZ = z - pendingDistanceI < 0 ? (int)backChunk.heightMap[x][32 - pendingDistanceI] : (int)chunk.heightMap[x][z - pendingDistanceI];
            int yPosZ = z + pendingDistanceI > 31 ? (int)frontChunk.heightMap[x][z + pendingDistanceI - chunk.widths] : (int)chunk.heightMap[x][z + pendingDistanceI];
            
            int leftDist = x - pendingDistanceI < 0 ? leftChunk.distanceI : pendingDistanceI;
            int rightDist = x + pendingDistanceI > 31 ? rightChunk.distanceI : pendingDistanceI;
            int backDist = z - pendingDistanceI < 0 ? backChunk.distanceI : pendingDistanceI;
            int frontDist = z + pendingDistanceI > 31 ? frontChunk.distanceI : pendingDistanceI;

            if (yNegX >= 0) yNegX = (yNegX / leftDist) * leftDist;
            if (yPosX >= 0) yPosX = (yPosX / rightDist) * rightDist;
            if (yNegZ >= 0) yNegZ = (yNegZ / backDist) * backDist;
            if (yPosZ >= 0) yPosZ = (yPosZ / frontDist) * frontDist;

            if(y >= 0){
            if(leftChunk.distanceI < pendingDistanceI && x == 0) yNegX-=leftChunk.distanceI;
            if(rightChunk.distanceI < pendingDistanceI && x == chunk.widths - pendingDistanceI) yPosX-=rightChunk.distanceI;
            if(backChunk.distanceI < pendingDistanceI && z == 0) yNegZ-=backChunk.distanceI;
            if(frontChunk.distanceI < pendingDistanceI && z == chunk.widths - pendingDistanceI) yPosZ-=frontChunk.distanceI;
            unsigned int current = 0;

#define addVertices(exposed)\
for (unsigned int i = (6 * exposed); i < (6 * exposed) + 6; i++) {\
    unsigned int tempVertice = naturalTiles[current].data[i];\
    tempVertice |= static_cast<unsigned int>(x + chunk.widths * (z + chunk.widths * y)) << 16;\
    verticesTemp.push_back(tempVertice);\
}\

            // y-min (face 0)
            // y-max (face 1)
            
            if(y != 32){
                current = 2; //grass
                addVertices(1);
                if(y > yNegX) addVertices(2);
                if(y > yPosX) addVertices(3);
                if(y > yNegZ) addVertices(4);
                if(y > yPosZ) addVertices(5);
                y-=pendingDistanceI;
                if(y < 0) continue;
                current = 3; //dirt
                if(y > yNegX) addVertices(2);
                if(y > yPosX) addVertices(3);
                if(y > yNegZ) addVertices(4);
                if(y > yPosZ) addVertices(5);
                y-=pendingDistanceI;
                if(y < 0) continue;
                while(y >= 0){
                    current = 1; //stone
                    if(y > yNegX) addVertices(2);
                    if(y > yPosX) addVertices(3);
                    if(y > yNegZ) addVertices(4);
                    if(y > yPosZ) addVertices(5);
                    y-=pendingDistanceI;
                }
            }else{
                y-=pendingDistanceI;
                int currentHeight = 0;
                if (noiseKey != noiseSearch.end()) {
                    auto& noiseData = noiseKey->second;
                    int localX = static_cast<int>(X - roundX); 
                    int localZ = static_cast<int>(Z - roundZ);

                    // Calculate current regional coordinate for this specific vertex loop step
                    float currentRegionX = static_cast<float>(localX + x);
                    float currentRegionZ = static_cast<float>(localZ + z);

                    // Find bounding indices in the 9x9 noise grid (spacing is 8 units)
                    int x0 = std::min(8, static_cast<int>(std::floor(currentRegionX / 8.0f)));
                    int x1 = std::min(8, x0 + 1);
                    float tX = (currentRegionX / 8.0f) - x0;

                    int z0 = std::min(8, static_cast<int>(std::floor(currentRegionZ / 8.0f)));
                    int z1 = std::min(8, z0 + 1);
                    float tZ = (currentRegionZ / 8.0f) - z0;

                    // Sample the 4 surrounding corners from the 9x9 grid
                    float n00 = noiseData[z0 * 9 + x0]; 
                    float n10 = noiseData[z0 * 9 + x1]; 
                    float n01 = noiseData[z1 * 9 + x0]; 
                    float n11 = noiseData[z1 * 9 + x1]; 

                    // Perform bilinear interpolation
                    float noise0 = n00 + tX * (n10 - n00); 
                    float noise1 = n01 + tX * (n11 - n01); 
                    float interpolatedNoise = noise0 + tZ * (noise1 - noise0);

                    // Replace your old "int currentHeight = std::floor(noiseKey->second[regionRowOffset]);" with this:
                    int currentHeight = std::floor(interpolatedNoise);
                }
                if(currentHeight - (Y + chunk.widths) == 0){
                    current = 3; //dirt
                    if(y > yNegX) addVertices(2);
                    if(y > yPosX) addVertices(3);
                    if(y > yNegZ) addVertices(4);
                    if(y > yPosZ) addVertices(5);
                    y-=pendingDistanceI;
                }
                current = 1; //stone
                while(y >= 0){
                    if(y > yNegX) addVertices(2);
                    if(y > yPosX) addVertices(3);
                    if(y > yNegZ) addVertices(4);
                    if(y > yPosZ) addVertices(5);
                    y-=pendingDistanceI;
                }
            }    
            } //y >= 0
        }
    }

    this->pendingVertices = std::move(verticesTemp);
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
