#ifndef SHADER_H
#define SHADER_H

#include "stb_image.h"
#include "cameraClass.h"
#include <fstream>
#include <sstream>

class Shader
{
public:
    unsigned int ID;
    // constructor generates the shader on the fly
    // ------------------------------------------------------------------------
    Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath = nullptr)
    {
        std::string vertexCode;
        std::string fragmentCode;
        std::string geometryCode;
        std::ifstream vShaderFile;
        std::ifstream fShaderFile;
        std::ifstream gShaderFile;
        vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        gShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        try
        {
            vShaderFile.open(vertexPath);
            fShaderFile.open(fragmentPath);
            std::stringstream vShaderStream, fShaderStream;
            vShaderStream << vShaderFile.rdbuf();
            fShaderStream << fShaderFile.rdbuf();
            vShaderFile.close();
            fShaderFile.close();
            vertexCode = vShaderStream.str();
            fragmentCode = fShaderStream.str();
            if (geometryPath != nullptr)
            {
                gShaderFile.open(geometryPath);
                std::stringstream gShaderStream;
                gShaderStream << gShaderFile.rdbuf();
                gShaderFile.close();
                geometryCode = gShaderStream.str();
            }
        }
        catch (std::ifstream::failure& e)
        {
            std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << std::endl;
        }
        const char* vShaderCode = vertexCode.c_str();
        const char* fShaderCode = fragmentCode.c_str();
        // 2. compile shaders
        unsigned int vertex, fragment;
        // vertex shader
        vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &vShaderCode, NULL);
        glCompileShader(vertex);
        checkCompileErrors(vertex, "VERTEX");
        // fragment Shader
        fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &fShaderCode, NULL);
        glCompileShader(fragment);
        checkCompileErrors(fragment, "FRAGMENT");
        // if geometry shader is given, compile geometry shader
        unsigned int geometry;
        if (geometryPath != nullptr)
        {
            const char* gShaderCode = geometryCode.c_str();
            geometry = glCreateShader(GL_GEOMETRY_SHADER);
            glShaderSource(geometry, 1, &gShaderCode, NULL);
            glCompileShader(geometry);
            checkCompileErrors(geometry, "GEOMETRY");
        }
        // shader Program
        ID = glCreateProgram();
        glAttachShader(ID, vertex);
        glAttachShader(ID, fragment);
        if (geometryPath != nullptr)
            glAttachShader(ID, geometry);
        glLinkProgram(ID);
        checkCompileErrors(ID, "PROGRAM");
        // delete the shaders as they're linked into our program now and no longer necessary
        glDeleteShader(vertex);
        glDeleteShader(fragment);
        if (geometryPath != nullptr)
            glDeleteShader(geometry);
    }

    // activate the shader
    // ------------------------------------------------------------------------
    void use()
    {
        glUseProgram(ID);
    }
    void Delete()
    {
        glDeleteProgram(ID);
    }
    // utility uniform functions
    // ------------------------------------------------------------------------
    void setBool(const std::string& name, bool value) const
    {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
    }
    // ------------------------------------------------------------------------
    void setInt(const std::string& name, int value) const
    {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
    }
    // ------------------------------------------------------------------------
    void setFloat(const std::string& name, float value) const
    {
        glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
    }
    // ------------------------------------------------------------------------
    void setVec2(const std::string& name, const glm::vec2& value) const
    {
        glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
    }
    void setVec2(const std::string& name, float x, float y) const
    {
        glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y);
    }
    // ------------------------------------------------------------------------
    void setVec3(const std::string& name, const glm::vec3& value) const
    {
        glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
    }
    void setVec3(const std::string& name, float x, float y, float z) const
    {
        glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
    }
    // ------------------------------------------------------------------------
    void setVec4(const std::string& name, const glm::vec4& value) const
    {
        glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
    }
    void setVec4(const std::string& name, float x, float y, float z, float w)
    {
        glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w);
    }
    // ------------------------------------------------------------------------
    void setMat2(const std::string& name, const glm::mat2& mat) const
    {
        glUniformMatrix2fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }
    // ------------------------------------------------------------------------
    void setMat3(const std::string& name, const glm::mat3& mat) const
    {
        glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }
    // ------------------------------------------------------------------------
    void setMat4(const std::string& name, const glm::mat4& mat) const
    {
        glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }

private:
    // utility function for checking shader compilation/linking errors.
    // ------------------------------------------------------------------------
    void checkCompileErrors(GLuint shader, std::string type)
    {
        GLint success;
        GLchar infoLog[1024];
        if (type != "PROGRAM")
        {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success)
            {
                glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            }
        }
        else
        {
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if (!success)
            {
                glGetProgramInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            }
        }
    }
};

class Texture {
public:
    unsigned int ID;
    unsigned char* data;
    Texture(const char* texturePaths[], short int textureNumber, GLint internalformat, bool atlas, int size) {
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

    };

};

class Chunk {
public:
    static const short widths = 40;
    const float voxelWidths = 0.25f;
    unsigned short objects[widths * widths * widths] = { 1 };
    bool empty = true;
    bool solid = true;
    float X = 0;
    float Y = 0;
    float Z = 0;
    short distanceI = 1;
    FastNoiseLite noise;
    FastNoiseLite largeNoise;
    Chunk(){
        initialNoiseSet();
    }

    Chunk(float X, float Y, float Z) {
        initialNoiseSet();
        create(X, Y, Z);
    }

    void initialNoiseSet(){
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
    float calcNoise(float x, float z, bool absolute) {
        if(!absolute){
            z = (z * voxelWidths + Z);
            x = (x * voxelWidths + X);
        }
        float currentNoise = 0.0f;
        if (storedNoise.count(glm::vec2(x, z)) == 1){
            currentNoise = storedNoise[glm::vec2(x, z)];
        }else{
            currentNoise = noise.GetNoise(x, z);
            float plusNoise = largeNoise.GetNoise(x, z);
            plusNoise *= 300.0f;
            currentNoise *= 12.0f;
            currentNoise = currentNoise * currentNoise;
            currentNoise -= (sin(x) * cos(z)) * (currentNoise / 70);
            currentNoise += plusNoise;
            currentNoise = abs(currentNoise);
            storedNoise[glm::vec2(x, z)] = currentNoise;
        }
        return currentNoise;
    }

    void create(float X, float Y, float Z) {
        this->X = X;
        this->Y = Y;
        this->Z = Z;
        this->solid = true;
        this->empty = true;
        //30 chunks: 16.315    PB: 16.315
        //12 chunks: 6.905     PB: 6.905
        if(debug.useLOD){
            distanceI = neighborDistanceI(X, Y, Z, 0.0f, 0.0f, 0.0f);
        }else{
            distanceI = 1;
        }
        short heights[40][40];
        for (int z = 0; z < widths; z+=distanceI) {
            for (int x = 0; x < widths; x+=distanceI) {
                float currentNoise = calcNoise(x + distanceI / 2, z + distanceI / 2, 0);
                heights[x][z] = -1;
                for (int y = 0; y < widths; y+=distanceI) {
                    
                    unsigned short blockType = (currentNoise - Y - (widths * voxelWidths) + ((widths - (y - 1)) * voxelWidths) + floor(widths * voxelWidths) > -1.0f);
                    this->objects[x + this->widths * (z + this->widths * y)] = blockType;
                    if(blockType == 1 && (currentNoise - Y - (widths * voxelWidths) + ((widths - (y + distanceI)) * voxelWidths) + floor(widths * voxelWidths) <= -1.0f)) heights[x][z] = y;
                    if(this->empty || this->solid){
                        if (blockType != 0){
                            this->empty = false;
                            if(this->solid == true){
                                if(distanceI == 40){
                                    if(!(currentNoise - Y - (widths * voxelWidths) + ((widths - (y - 1 - distanceI)) * voxelWidths) + floor(widths * voxelWidths) > -1.0f)) this->solid = false;
                                    if(!(currentNoise - Y - (widths * voxelWidths) + ((widths - (y + distanceI)) * voxelWidths) + floor(widths * voxelWidths) > -1.0f)) this->solid = false;
                                    float tempNoise = calcNoise(x + distanceI / 2 - distanceI, z + distanceI / 2, 0);
                                    if(!(tempNoise - Y - (widths * voxelWidths) + ((widths - (y - 1)) * voxelWidths) + floor(widths * voxelWidths) > -1.0f)) this->solid = false;
                                    tempNoise = calcNoise(x + distanceI / 2 + distanceI, z + distanceI / 2, 0);
                                    if(!(tempNoise - Y - (widths * voxelWidths) + ((widths - (y - 1)) * voxelWidths) + floor(widths * voxelWidths) > -1.0f)) this->solid = false;
                                    tempNoise = calcNoise(x + distanceI / 2, z + distanceI / 2 - distanceI, 0);
                                    if(!(tempNoise - Y - (widths * voxelWidths) + ((widths - (y - 1)) * voxelWidths) + floor(widths * voxelWidths) > -1.0f)) this->solid = false;
                                    tempNoise = calcNoise(x + distanceI / 2, z + distanceI / 2 + distanceI, 0);
                                    if(!(tempNoise - Y - (widths * voxelWidths) + ((widths - (y - 1)) * voxelWidths) + floor(widths * voxelWidths) > -1.0f)) this->solid = false;
                                }else{
                                    if(y == 0){
                                        if(!(currentNoise - Y - (widths * voxelWidths) + ((widths - (y - 1 - distanceI)) * voxelWidths) + floor(widths * voxelWidths) > -1.0f)) this->solid = false;
                                    }
                                    if(y == widths - distanceI){
                                        if(!(currentNoise - Y - (widths * voxelWidths) + ((widths - (y + distanceI)) * voxelWidths) + floor(widths * voxelWidths) > -1.0f)) this->solid = false;
                                    }
                                    if(x == 0){
                                        float tempNoise = calcNoise(x - distanceI, z, 0);
                                        if(!(tempNoise - Y - (widths * voxelWidths) + ((widths - (y - 1)) * voxelWidths) + floor(widths * voxelWidths) > -1.0f)) this->solid = false;
                                    }
                                    if(x == widths - distanceI){
                                        float tempNoise = calcNoise(x + distanceI, z, 0);
                                        if(!(tempNoise - Y - (widths * voxelWidths) + ((widths - (y - 1)) * voxelWidths) + floor(widths * voxelWidths) > -1.0f)) this->solid = false;
                                    }
                                    if(z == 0){
                                        float tempNoise = calcNoise(x, z - distanceI, 0);
                                        if(!(tempNoise - Y - (widths * voxelWidths) + ((widths - (y - 1)) * voxelWidths) + floor(widths * voxelWidths) > -1.0f)) this->solid = false;
                                    }
                                    if(z == widths - distanceI){
                                        float tempNoise = calcNoise(x, z + distanceI, 0);
                                        if(!(tempNoise - Y - (widths * voxelWidths) + ((widths - (y - 1)) * voxelWidths) + floor(widths * voxelWidths) > -1.0f)) this->solid = false;
                                    }
                                }
                            }
                        }else{
                            this->solid = false;
                        }
                    }
                }
            }
        }

        for (int z = 0; z < widths; z+=distanceI) {
            for (int x = 0; x < widths; x+=distanceI) {
                if(heights[x][z] != -1){
                    this->objects[x + this->widths * (z + this->widths * heights[x][z])] = 2;
                    if(heights[x][z] - 1 >= 0) this->objects[x + this->widths * (z + this->widths * (heights[x][z] - 1))] = 3;
                    if(heights[x][z] - 2 >= 0 && rand() % 3 > 0) this->objects[x + this->widths * (z + this->widths * (heights[x][z] - 2))] = 3;
                    if(heights[x][z] - 3 >= 0 && rand() % 3 > 1) this->objects[x + this->widths * (z + this->widths * (heights[x][z] - 3))] = 3;
                }
            }
        }
    }

    unsigned short closeBlock(int offsetX, float offsetY, int offsetZ, short distanceI2) {
        float currentNoise = calcNoise(offsetX + distanceI2 / 2, offsetZ + distanceI2 / 2, 0);
        return static_cast<unsigned short>((currentNoise - Y - (widths * voxelWidths) + ((widths - (offsetY - 1)) * voxelWidths) + floor(widths * voxelWidths) > -1.0f));
    }

    short neighborDistanceI(float chunkX, float chunkY, float chunkZ, float xoffset, float yoffset, float zoffset) {
        short neighborDistance = trunc(sqrt(((chunkX + xoffset) - generatePos.x) * ((chunkX + xoffset) - generatePos.x) + ((chunkY + yoffset) - generatePos.y) * ((chunkY + yoffset) - generatePos.y) + ((chunkZ + zoffset) - generatePos.z) * ((chunkZ + zoffset) - generatePos.z))) / 30;
        if (neighborDistance > 15) neighborDistance = 15;
        return distanceIncriment[neighborDistance];
    }

    ~Chunk() {

    }
private:
};


class Mesh {
public:
    float X = 0.0f;
    float Y = 0.0f;
    float Z = 0.0f;
    unsigned int* vertices;
    unsigned int length;
    unsigned int* indices = 0u;
    unsigned int indicesAmount = 0u;
    unsigned int VAO, VBO, EBO;
    short distanceI = 1;

    Mesh() {
        this->length = 0;
        this->vertices = new unsigned int[0];
        this->indices = new unsigned int[0];
        this->X = 0.0f;
        this->Y = 0.0f;
        this->Z = 0.0f;
        glGenVertexArrays(1, &this->VAO);
        glGenBuffers(1, &this->VBO);
        //glGenBuffers(1, &this->EBO);
    }

    Mesh(unsigned int size, unsigned int items[], Chunk& chunk) {
        this->length = size;
        this->vertices = new unsigned int[size];
        for (unsigned int i = 0; i < size; i++) {
            this->vertices[i] = items[i];
        }
        this->indices = new unsigned int[0];
        this->X = 0.0f;
        this->Y = 0.0f;
        this->Z = 0.0f;
        this->X = chunk.X;
        this->Y = chunk.Y;
        this->Z = chunk.Z;
        glGenVertexArrays(1, &this->VAO);
        glGenBuffers(1, &this->VBO);
        //glGenBuffers(1, &this->EBO);
        this->fillChunk(chunk);
    }

    ~Mesh() {
        delete[] this->vertices;
        delete[] this->indices;
        this->vertices = NULL;
        this->indices = NULL;
        glDeleteVertexArrays(1, &this->VAO);
        glDeleteBuffers(1, &this->VBO);
        //glDeleteBuffers(1, &this->EBO);
    }

    void updateBuffers() {
        glBindVertexArray(this->VAO);
        glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
        glBufferData(GL_ARRAY_BUFFER, this->size(0), this->vertices, GL_DYNAMIC_DRAW);
        //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);
        //glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->size(1), this->indices, GL_DYNAMIC_DRAW);
        glVertexAttribIPointer(0, 1, GL_UNSIGNED_INT, sizeof(unsigned int), (void*)0);
        glEnableVertexAttribArray(0);
    }

    void listItems(bool which) {
        if (!which) {
            std::cout << "***********VERTICES LIST***********" << '\n';
            std::cout << "length: " << this->length << '\n';
            for (unsigned int i = 0; i < this->length; i++) {
                std::cout << this->vertices[i] << " (Binary: " << std::bitset<32>(this->vertices[i]) << ")" << '\n';
            }
        }
        else {
            std::cout << "***********INDICES LIST***********" << '\n';
            std::cout << "length: " << this->indicesAmount << '\n';
            for (unsigned int i = 0; i < this->indicesAmount; i++) {
                std::cout << this->indices[i] << ", ";
                if (i % 3 == 2) std::cout << '\n';
            }
        }
    }

    unsigned int size(bool type) {
        //std::cout << this->length * sizeof(unsigned int);
        if (!type) {
            return this->length * sizeof(unsigned int);
        }
        else {
            return (this->indicesAmount) * sizeof(unsigned int);
        }
    }

    void setup(unsigned int size, unsigned int items[]) {
        this->length = size;
        delete[] this->vertices;
        this->vertices = NULL;
        this->vertices = new unsigned int[size];
        for (unsigned int i = 0; i < size; i++) {
            this->vertices[i] = items[i];
        }
        updateBuffers();
    }

    

    void fillChunk(Chunk& chunk) {
        delete[] this->vertices;
        this->vertices = NULL;
        this->distanceI = chunk.distanceI;
        if (chunk.empty == false && chunk.solid == false) {
            short otherLODchunks[6] = { 0 };
            otherLODchunks[0] = chunk.neighborDistanceI(chunk.X, chunk.Y, chunk.Z, 0, -10, 0);
            otherLODchunks[1] = chunk.neighborDistanceI(chunk.X, chunk.Y, chunk.Z, 0, 10, 0);
            otherLODchunks[2] = chunk.neighborDistanceI(chunk.X, chunk.Y, chunk.Z, -10, 0, 0);
            otherLODchunks[3] = chunk.neighborDistanceI(chunk.X, chunk.Y, chunk.Z, 10, 0, 0);
            otherLODchunks[4] = chunk.neighborDistanceI(chunk.X, chunk.Y, chunk.Z, 0, 0, -10);
            otherLODchunks[5] = chunk.neighborDistanceI(chunk.X, chunk.Y, chunk.Z, 0, 0, 10);

            std::vector<unsigned int> verticesTemp;
            for (int y = 0; y < chunk.widths; y += this->distanceI) {
                for (int z = 0; z < chunk.widths; z += this->distanceI) {
                    for (int x = 0; x < chunk.widths; x += this->distanceI) {
                        if (chunk.objects[x + chunk.widths * (z + chunk.widths * y)] != 0) {
                            short current = chunk.objects[x + chunk.widths * (z + chunk.widths * y)];
                            // y-min (face 0)
                            bool exposedFaces[6] = { false };

#define addVertices(exposed)\
if(exposedFaces[exposed] == false){\
for (unsigned int i = (6 * exposed); i < (6 * exposed) + 6; i++) {\
    unsigned int tempVertice = naturalTiles[current].data[i];\
    tempVertice |= static_cast<unsigned int>(x + chunk.widths * (z + chunk.widths * y)) << 16;\
    verticesTemp.push_back(tempVertice);\
}\
exposedFaces[exposed] = true;\
}\

                        if(debug.useLOD){
                            // y-min (face 0)
                            if(y > 0){
                                if (chunk.objects[x + chunk.widths * (z + chunk.widths * (y - distanceI))] == 0) addVertices(0);
                            }else{
                                for(int xL = 0; xL < distanceI; xL++){
                                    for(int zL = 0; zL < distanceI; zL++){
                                        if(chunk.closeBlock(x + xL, -1, z + zL, otherLODchunks[0]) == 0) addVertices(0);
                                        goto f0exit;
                                    }
                                }
                            }
                            f0exit: NULL;
                            // y-max (face 1)
                            if(y < chunk.widths - distanceI){
                                if (chunk.objects[x + chunk.widths * (z + chunk.widths * (y + distanceI))] == 0) addVertices(1);
                            }else{
                                for(int xL = 0; xL < distanceI; xL++){
                                    for(int zL = 0; zL < distanceI; zL++){
                                        if(chunk.closeBlock(x + xL, y + distanceI, z + zL, otherLODchunks[1]) == 0) addVertices(1);
                                        goto f1exit;
                                    }
                                }
                            }
                            f1exit: NULL;
                            // x-min (face 2)
                            if(x > 0){
                                if (chunk.objects[(x - distanceI) + chunk.widths * (z + chunk.widths * y)] == 0) addVertices(2);
                            }else{
                                for(int yL = 0; yL < distanceI; yL++){
                                    for(int zL = 0; zL < distanceI; zL++){
                                        if(chunk.closeBlock(-distanceI, y + yL, z + zL, otherLODchunks[2]) == 0) addVertices(2);
                                        goto f2exit;
                                    }
                                }
                            }
                            f2exit: NULL;
                            // x-max (face 3)
                            if(x < chunk.widths - distanceI){
                                if (chunk.objects[(x + distanceI) + chunk.widths * (z + chunk.widths * y)] == 0) addVertices(3);
                            }else{
                                for(int yL = 0; yL < distanceI; yL++){
                                    for(int zL = 0; zL < distanceI; zL++){
                                        if(chunk.closeBlock(x + distanceI, y + yL, z + zL, otherLODchunks[3]) == 0) addVertices(3);
                                        goto f3exit;
                                    }
                                }
                            }
                            f3exit: NULL;
                            // z-min (face 4)
                            if(z > 0){
                                if (chunk.objects[x + chunk.widths * ((z - distanceI) + chunk.widths * y)] == 0) addVertices(4);
                            }else{
                                for(int yL = 0; yL < distanceI; yL++){
                                    for(int xL = 0; xL < distanceI; xL++){
                                        if(chunk.closeBlock(x + xL, y + yL, -distanceI, otherLODchunks[4]) == 0) addVertices(4);
                                        goto f4exit;
                                    }
                                }
                            }
                            f4exit: NULL;
                            // z-max (face 5)
                            if(z < chunk.widths - distanceI){
                                if (chunk.objects[x + chunk.widths * ((z + distanceI) + chunk.widths * y)] == 0) addVertices(5);
                            }else{
                                for(int yL = 0; yL < distanceI; yL++){
                                    for(int xL = 0; xL < distanceI; xL++){
                                        if(chunk.closeBlock(x + xL, y + yL, z + distanceI, otherLODchunks[5]) == 0) addVertices(5);
                                        goto f5exit;
                                    }
                                }
                            }
                            f5exit: NULL;
                            
                        }else{
                            //no LOD
                            if (y > 0) {
                                if (chunk.objects[x + chunk.widths * (z + chunk.widths * (y - 1))] == 0) addVertices(0);
                            }
                            else {
                                if (chunk.closeBlock(x, -1, z, 1) == 0) addVertices(0);
                            }
                            if (y < chunk.widths - 1) {
                                if (chunk.objects[x + chunk.widths * (z + chunk.widths * (y + 1))] == 0) addVertices(1);
                            }
                            else {
                                if (chunk.closeBlock(x, 40, z, 1) == 0) addVertices(1);
                            }
                            if (x > 0) {
                                if (chunk.objects[(x - 1) + chunk.widths * (z + chunk.widths * y)] == 0) addVertices(2);
                            }
                            else {
                                if (chunk.closeBlock(-1, y, z, 1) == 0) addVertices(2);
                            }
                            if (x < chunk.widths - 1) {
                                if (chunk.objects[(x + 1) + chunk.widths * (z + chunk.widths * y)] == 0) addVertices(3);
                            }
                            else {
                                if (chunk.closeBlock(40, y, z, 1) == 0) addVertices(3);
                            }
                            if (z > 0) {
                                if (chunk.objects[x + chunk.widths * ((z - 1) + chunk.widths * y)] == 0) addVertices(4);
                            }
                            else {
                                if (chunk.closeBlock(x, y, -1, 1) == 0) addVertices(4);
                            }
                            if (z < chunk.widths - 1) {
                                if (chunk.objects[x + chunk.widths * ((z + 1) + chunk.widths * y)] == 0) addVertices(5);
                            }
                            else {
                                if (chunk.closeBlock(x, y, 40, 1) == 0) addVertices(5);
                            }
                        }
                            
                        }
                    }
                }
            }

            unsigned int newSize = static_cast<unsigned int>(verticesTemp.size());
            this->vertices = new unsigned int[newSize];
            this->length = newSize;
            for (unsigned int i = 0; i < newSize; i++) {
                if (i < this->length) this->vertices[i] = verticesTemp[i];
            }
            //updateIndices();
        } else {
            this->vertices = new unsigned int[0];
            this->length = 0;
        }
        this->X = chunk.X;
        this->Y = chunk.Y;
        this->Z = chunk.Z;
        bytesFromMeshes += (length * 4.f) + 34.f;
        totalChunksGenerated++;
    }

private:

    void updateIndices() {
        delete[] this->indices;
        this->indices = NULL;
        std::vector<unsigned int>uniqueVertices;
        unsigned int* vertexIndexes;
        vertexIndexes = new unsigned int[this->length];
        unsigned int count = 0;
        for (unsigned int i = 0; i < this->length; i++) {
            auto found = std::find(uniqueVertices.begin(), uniqueVertices.end(), this->vertices[i]);
            if (found == uniqueVertices.end()) {
                uniqueVertices.push_back(this->vertices[i]);
                vertexIndexes[i] = count++;
            }
            else {
                vertexIndexes[i] = std::distance(uniqueVertices.begin(), found);
            }
        }
        indices = new unsigned int[this->length];
        for (unsigned int i = 0; i < this->length; i++) {
            this->indices[i] = vertexIndexes[i];
        }
        delete[] this->vertices;
        this->vertices = NULL;
        this->vertices = new unsigned int[uniqueVertices.size()];
        for (unsigned int i = 0; i < uniqueVertices.size(); i++) {
            this->vertices[i] = uniqueVertices[i];
        }
        this->indicesAmount = this->length;
        this->length = uniqueVertices.size();
    }

};


#endif