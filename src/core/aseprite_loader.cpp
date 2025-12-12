#include "aseprite_loader.hpp"
#include "stb_image.h"
#include "log.hpp"
#include <fstream>
#include <cstring>
#include <cstdint>
#include <cstdlib>

namespace yuki {
namespace {
uint16_t rd16(const unsigned char* p) { return (uint16_t)p[0] | ((uint16_t)p[1] << 8); }
uint32_t rd32(const unsigned char* p) { return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24); }
int16_t rds16(const unsigned char* p) { return (int16_t)rd16(p); }

bool readPascalString(const unsigned char* data, size_t size, size_t& off, std::string& out) {
    if (off + 2 > size) return false;
    uint16_t len = rd16(data + off);
    off += 2;
    if (off + len > size) return false;
    out.assign((const char*)(data + off), len);
    off += len;
    return true;
}
}

bool loadAsepriteFile(const std::string& path, AseData& out) {
    std::ifstream file(path, std::ios::binary);
    if (!file) return false;
    std::vector<unsigned char> bytes((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    if (bytes.size() < 128) return false;
    uint16_t magic = rd16(bytes.data() + 4);
    if (magic != 0xA5E0) return false;
    uint16_t frameCount = rd16(bytes.data() + 6);
    uint16_t w = rd16(bytes.data() + 8);
    uint16_t h = rd16(bytes.data() + 10);
    uint16_t colorDepth = rd16(bytes.data() + 12);
    if (colorDepth != 32) return false;
    out.width = w;
    out.height = h;
    out.frames.resize(frameCount);
    out.durationsMs.resize(frameCount);
    size_t offset = 128;
    for (int fi = 0; fi < frameCount; ++fi) {
        if (offset + 16 > bytes.size()) return false;
        uint32_t frameBytes = rd32(bytes.data() + offset);
        uint16_t frameMagic = rd16(bytes.data() + offset + 4);
        uint16_t chunkCount = rd16(bytes.data() + offset + 6);
        uint16_t duration = rd16(bytes.data() + offset + 8);
        size_t frameStart = offset;
        offset += 16;
        out.durationsMs[fi] = (int)duration;
        if (chunkCount == 0xFFFF) {
            if (offset + 4 > bytes.size()) return false;
            chunkCount = rd32(bytes.data() + offset);
            offset += 4;
        }
        if (frameMagic != 0xF1FA) return false;
        std::vector<unsigned char> canvas((size_t)w * (size_t)h * 4, 0);
        for (int ci = 0; ci < chunkCount; ++ci) {
            if (offset + 6 > bytes.size()) return false;
            uint32_t chunkSize = rd32(bytes.data() + offset);
            uint16_t chunkType = rd16(bytes.data() + offset + 4);
            size_t chunkStart = offset;
            offset += 6;
            if (chunkStart + chunkSize > bytes.size()) return false;
            if (chunkType == 0x2018) {
                if (offset + 10 > bytes.size()) return false;
                uint16_t tagCount = rd16(bytes.data() + offset);
                offset += 2 + 8;
                for (int ti = 0; ti < tagCount; ++ti) {
                    if (offset + 17 > bytes.size()) return false;
                    uint16_t from = rd16(bytes.data() + offset);
                    uint16_t to = rd16(bytes.data() + offset + 2);
                    uint8_t direction = bytes[offset + 4];
                    offset += 17;
                    std::string name;
                    if (!readPascalString(bytes.data(), bytes.size(), offset, name)) return false;
                    AseTag tag;
                    tag.name = name;
                    tag.from = from;
                    tag.to = to;
                    tag.direction = direction;
                    out.tags.push_back(tag);
                }
                offset = chunkStart + chunkSize;
            } else if (chunkType == 0x2005) {
                if (offset + 16 > bytes.size()) return false;
                uint16_t layerIndex = rd16(bytes.data() + offset);
                int16_t cx = rds16(bytes.data() + offset + 2);
                int16_t cy = rds16(bytes.data() + offset + 4);
                uint8_t opacity = bytes[offset + 6];
                uint16_t celType = rd16(bytes.data() + offset + 7);
                offset += 16;
                (void)layerIndex;
                if (celType == 0 || celType == 2) {
                    if (offset + 4 > bytes.size()) return false;
                    uint16_t cw = rd16(bytes.data() + offset);
                    uint16_t ch = rd16(bytes.data() + offset + 2);
                    offset += 4;
                    size_t pixCount = (size_t)cw * (size_t)ch * 4;
                    std::vector<unsigned char> celPixels;
                    if (celType == 0) {
                        if (offset + pixCount > bytes.size()) return false;
                        celPixels.assign(bytes.begin() + offset, bytes.begin() + offset + pixCount);
                        offset += pixCount;
                    } else {
                        size_t compSize = chunkStart + chunkSize - offset;
                        int outLen = 0;
                        char* raw = stbi_zlib_decode_malloc((const char*)(bytes.data() + offset), (int)compSize, &outLen);
                        if (!raw) return false;
                        celPixels.assign((unsigned char*)raw, (unsigned char*)raw + outLen);
                        free(raw);
                        offset += compSize;
                        if (celPixels.size() < pixCount) return false;
                    }
                    for (uint16_t iy = 0; iy < ch; ++iy) {
                        for (uint16_t ix = 0; ix < cw; ++ix) {
                            size_t src = ((size_t)iy * cw + ix) * 4;
                            int dstx = cx + ix;
                            int dsty = cy + iy;
                            if (dstx < 0 || dsty < 0 || dstx >= w || dsty >= h) continue;
                            size_t dst = ((size_t)dsty * w + dstx) * 4;
                            unsigned char b = celPixels[src + 0];
                            unsigned char g = celPixels[src + 1];
                            unsigned char r = celPixels[src + 2];
                            unsigned char a = (unsigned char)((celPixels[src + 3] * opacity) / 255);
                            float af = a / 255.0f;
                            float inv = 1.0f - af;
                            canvas[dst + 0] = (unsigned char)(r * af + canvas[dst + 0] * inv);
                            canvas[dst + 1] = (unsigned char)(g * af + canvas[dst + 1] * inv);
                            canvas[dst + 2] = (unsigned char)(b * af + canvas[dst + 2] * inv);
                            canvas[dst + 3] = (unsigned char)(a + canvas[dst + 3] * inv);
                        }
                    }
                } else if (celType == 1) {
                    offset += 2;
                }
                offset = chunkStart + chunkSize;
            } else {
                offset = chunkStart + chunkSize;
            }
        }
        out.frames[fi] = std::move(canvas);
        offset = frameStart + frameBytes;
        if (offset > bytes.size()) return false;
    }
    if (out.tags.empty()) {
        AseTag t;
        t.name = "default";
        t.from = 0;
        t.to = (int)out.frames.size() - 1;
        t.direction = 0;
        out.tags.push_back(t);
    }
    return true;
}
}
