#include <vector>
#include <cstdint>
#include <glm/gtc/type_ptr.hpp>

struct Node {
    glm::vec3 pos;

    uint8_t r = 0, g = 0, b = 0;

    uint8_t material = 0;

    bool operator == (const Node& other) const{
        return other.pos == pos;
    }
};

std::vector<Node> nodePool;

/*void updateNodeAverages(uint32_t parentIndex) {
    Node& parent = nodePool[parentIndex];
    if (parent.childIndex == 0) return; // Is leaf
    int totalR = 0, totalG = 0, totalB = 0;
    int solidCount = 0;

    for (int i = 0; i < 8; i++) {
        Node& child = nodePool[parent.childIndex + i];
        if (child.material != 0) { // If not air
            totalR += child.r;
            totalG += child.g;
            totalB += child.b;
            solidCount++;
        }
    }

    if (solidCount > 0) {
        parent.r = totalR / solidCount;
        parent.g = totalG / solidCount;
        parent.b = totalB / solidCount;
        parent.material = 255; // 255 could mean "Mixed/Branch"
    } else {
        parent.material = 0; // Completely empty branch
    }
}*/

void splitNode(){

}