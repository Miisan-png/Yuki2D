#pragma once
#include <string>
#include <vector>
#include <unordered_map>

namespace yuki {
struct AseTag {
    std::string name;
    int from = 0;
    int to = 0;
    int direction = 0;
};

struct AseData {
    int width = 0;
    int height = 0;
    std::vector<std::vector<unsigned char>> frames;
    std::vector<int> durationsMs;
    std::vector<AseTag> tags;
};

bool loadAsepriteFile(const std::string& path, AseData& out);
}
