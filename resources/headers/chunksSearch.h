#ifndef CHUNKS_DECL_H
#define CHUNKS_DECL_H

#include <unordered_map>
#include <glm/glm.hpp>
#include <functional>

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

extern std::unordered_map<glm::vec3, unsigned int, vec3Hash> chunksSearch;

#endif
