#include "binarymask.hpp"
#include <cstring>
#include <iostream>
#include <stdexcept>

BinaryMaskData::BinaryMaskData(size_t width2, size_t height2) : width(width2), height(height2) {
    size_t totalPixels = width * height;
    size_t bytes = ((totalPixels + 7) / 8);
    data.resize(bytes, 0x00);
}

bool BinaryMaskData::sample(size_t x, size_t y) const {
    const size_t index = y * width + x;
    const size_t byteIndex = index >> 3;
    const uint8_t bitMask = static_cast<uint8_t>(1u << (index & 7));
    return (data[byteIndex] & bitMask) != 0;
}

void BinaryMaskData::set(size_t x, size_t y, bool value) {
    const size_t index = y * width + x;
    const size_t byteIndex = index >> 3;
    const uint8_t bitMask = static_cast<uint8_t>(1u << (index & 7));
    data[byteIndex] = (data[byteIndex] & ~bitMask) | (-static_cast<uint8_t>(value) & bitMask);
}
void BinaryMaskData::crop(Vec2 newMin, Vec2 newMax) {
    if (newMin.y > newMax.y || newMin.x > newMax.x) {
        throw std::invalid_argument("error in crop: min is less than max. min=(" + std::to_string(newMin.x) + "," + std::to_string(newMin.y) + "), max=(" + std::to_string(newMax.x) + "," + std::to_string(newMax.y) + ")");
    }
    BinaryMaskData newData(newMax.x - newMin.x, newMax.y - newMin.y);

    for (size_t y = 0; y < newData.height; y++) {
        for (size_t x = 0; x < newData.width; x++) {
            size_t srcX = x + newMin.x;
            size_t srcY = y + newMin.y;
            newData.set(x, y, (srcX < 0 || srcY < 0 || srcX >= width || srcY >= height) ? false : sample(srcX, srcY));
        }
    }
    *this = newData;
}
void BinaryMaskData::clearAll() {
    std::memset(data.data(), 0x00, data.size());
}

BinaryRasterMask::BinaryRasterMask(Vec2 min2, Vec2 max2) : min(min2), max(max2), data(max2.x - min2.x, max2.y - min2.y) {
}
Vec2 BinaryRasterMask::getMin() const {
    return min;
}
Vec2 BinaryRasterMask::getMax() const {
    return max;
}
size_t BinaryRasterMask::getWidth() const {
    return max.x - min.x;
}
size_t BinaryRasterMask::getHeight() const {
    return max.y - min.y;
}
void BinaryRasterMask::set(int x, int y, bool value) {
    if (x < min.x || y < min.y || x >= max.x || y >= max.y) return;
    data.set(x - min.x, y - min.y, value);
}
bool BinaryRasterMask::sample(int x, int y) const {
    if (x < min.x || y < min.y || x >= max.x || y >= max.y) return false;
    return data.sample(x - min.x, y - min.y);
}
void BinaryRasterMask::cropToContent() {
    int minX;
    int minY;
    int maxX;
    int maxY;
    // top
    bool topFound = false;
    for (size_t i = 0; i < data.data.size(); i++) {
        if (data.data[i] != 0) {
            size_t bitIndex = i * 8 + static_cast<size_t>(std::countl_zero(data.data[i]));
            int y = bitIndex / getWidth();
            minY = y;
            topFound = true;
            break;
        }
    }
    if (!topFound) return;

    // bottom
    bool bottomFound = false;
    for (long long i = data.data.size() - 1; i >= 0; i--) {
        if (data.data[i] != 0) {
            size_t bitIndex = i * 8 + static_cast<size_t>(std::countr_zero(data.data[i]));
            int y = bitIndex / getWidth();
            maxY = y + 1;
            bottomFound = true;
            break;
        }
    }
    if (!bottomFound) return;
    // make these better
    // left
    bool leftFound = false;
    for (size_t x = 0; x < getWidth(); x++) {
        for (size_t y = 0; y < getHeight(); y++) {
            if (data.sample(x, y)) {
                leftFound = true;
                minX = x;
                break;
            }
        }
        if (leftFound) break;
    }
    if (!leftFound) return;
    // right
    bool rightFound = false;
    for (long long x = getWidth() - 1; x >= 0; x--) {
        for (size_t y = 0; y < getHeight(); y++) {
            if (data.sample(x, y)) {
                rightFound = true;
                maxX = x + 1;
                break;
            }
        }
        if (rightFound) break;
    }
    if (!rightFound) return;

    min.x += minX;
    max.x = min.x + (maxX - minX);
    min.y += minY;
    max.y = min.y + (maxY - minY);

    data.crop({minX, minY}, {maxX, maxY});
}
void BinaryRasterMask::makeWidthAndHeightDivisibleBy(int x) {
    if (x <= 1) return;
    int widthToAdd = (x - (getWidth() % x)) % x;
    int heightToAdd = (x - (getHeight() % x)) % x;
    if (widthToAdd == 0 && heightToAdd == 0) return;

    int minXToAdd = widthToAdd / 2;
    int minYToAdd = heightToAdd / 2;
    int maxXToAdd = widthToAdd - minXToAdd;
    int maxYToAdd = heightToAdd - minYToAdd;
    std::cout << minXToAdd << minYToAdd << maxXToAdd << maxYToAdd << std::endl;

    min.x -= minXToAdd;
    min.y -= minYToAdd;
    max.x += maxXToAdd;
    max.y += maxYToAdd;

    data.crop({-minXToAdd, -minYToAdd}, {(int)(data.width + maxXToAdd), (int)(data.height + maxYToAdd)});
}
