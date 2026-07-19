#ifndef CHUNKS_DECL_H
#define CHUNKS_DECL_H

#include <unordered_map>
#include <glm/glm.hpp>
#include <functional>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

extern std::unordered_map<glm::vec3, unsigned int> chunksSearch;
extern std::unordered_map<glm::vec2, float[9*9]> noiseSearch;

#endif
