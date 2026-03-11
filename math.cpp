#include "math.hpp"
#include <cmath>

int clamp(int a, int b, int c) {
    return std::min(std::max(a, b), c);
}
float square(float x) {
    return x * x;
}
float inverseLerp(float x, float a, float b) {
    return (x - a) / (b - a);
}
uint8_t lerp(uint8_t a, uint8_t b, float t) {
    float result = std::lerp(static_cast<float>(a), static_cast<float>(b), t);
    if (result < 0.f) return 0;
    if (result > 255.f) return 255;
    return static_cast<uint8_t>(result);
}

Vec2 Vec2::operator-(const Vec2& other) {
    return {x - other.x, y - other.y};
}
Vec2 Vec2::operator+(const int& other) {
    return {x + other, y + other};
}

void Vec2f::operator+=(const Vec2f& other) {
    x += other.x;
    y += other.y;
}
Vec2f Vec2f::operator-(const Vec2f& other) {
    return {x - other.x, y - other.y};
}
Vec2f Vec2f::operator+(const float& other) {
    return {x + other, y + other};
}
Vec2f Vec2f::operator*(const float& other) {
    return {x * other, y * other};
}
Vec2f Vec2f::operator/(const float& other) {
    return {x / other, y / other};
}

void Vec3f::operator+=(const Vec3f& other) {
    x += other.x;
    y += other.y;
    z += other.z;
}
Vec3f Vec3f::operator-(const Vec3f& other) {
    return {x - other.x, y - other.y, z - other.z};
}
Vec3f Vec3f::operator+(const float& other) {
    return {x + other, y + other, z * other};
}
Vec3f Vec3f::operator*(const float& other) {
    return {x * other, y * other, z * other};
}
Vec3f Vec3f::operator/(const float& other) {
    return {x / other, y / other, z * other};
}
void Vec3f::normalize() {
    float invMagnitude = 1.f / std::sqrt(x * x + y * y + z * z);
    x *= invMagnitude;
    y *= invMagnitude;
    z *= invMagnitude;
}

NearestNeighbor::NearestNeighbor(int width, int height, std::vector<Vec2f>& points) : gridWidth(width), gridHeight(height) {
    minX = 0.f;
    maxX = 0.f;
    minY = 0.f;
    maxY = 0.f;
    bool first = true;
    for (Vec2f point : points) {
        if (first || point.x < minX) minX = point.x;
        if (first || point.y < minY) minY = point.y;
        if (first || point.x > minX) maxX = point.x;
        if (first || point.y > minY) maxY = point.y;
        first = false;
    }

    cells.resize(gridWidth * gridHeight);
    for (Vec2f point : points) {
        int cellX = clamp(static_cast<int>(inverseLerp(point.x, minX, maxX) * static_cast<float>(gridWidth)), 0, gridWidth - 1);
        int cellY = clamp(static_cast<int>(inverseLerp(point.y, minY, maxY) * static_cast<float>(gridHeight)), 0, gridHeight - 1);
        cells[cellY * gridWidth + cellX].push_back(point);
    }
}
Vec2f NearestNeighbor::getNearestNeighbor(Vec2f point) const {
    int cellX = clamp(static_cast<int>(inverseLerp(point.x, minX, maxX) * static_cast<float>(gridWidth)), 0, gridWidth - 1);
    int cellY = clamp(static_cast<int>(inverseLerp(point.y, minY, maxY) * static_cast<float>(gridHeight)), 0, gridHeight - 1);


    int checkWidth = 1;

    // loop over ring
    bool foundNothing = false;
    while (!foundNothing) { // 1 ring
        size_t n = checkWidth == 1 ? 1 : 4 * (checkWidth - 1); // number of points in ring
        int halfWidth = checkWidth / 2;
        Vec2 check{-halfWidth, -halfWidth}; // 5: 2, 3: 1, 1: 0
        for (size_t i = 0; i < n; i++) {
            bool found = false;
            int cellIndex = check.y * gridWidth + check.x;

            // check current cell
            bool first = true;
            Vec2f closestPoint;
            float closestSquareDistance;
            for (Vec2f cellPoint : cells[cellIndex]) {
                float squareDistance = (point.x - cellPoint.x) * (point.x - cellPoint.x) + (point.y - cellPoint.y) * (point.y - cellPoint.y);
                if (first || squareDistance < closestSquareDistance) {
                    closestSquareDistance = squareDistance;
                    first = false;
                }
            }

            // move clockwise
            if      (check.x == -halfWidth && check.y !=  halfWidth) check.y += 1; // left   side: move up
            else if (check.y ==  halfWidth && check.x !=  halfWidth) check.x += 1; // top    side: move right
            else if (check.x ==  halfWidth && check.y != -halfWidth) check.y -= 1; // right  side: move down
            else if (check.y == -halfWidth && check.x != -halfWidth) check.x -= 1; // bottom side: move left
        }
        checkWidth += 2;
    }
}


int Vec2iref::x() const {return getValue(xValue);}
int Vec2iref::y() const {return getValue(yValue);}
Vec2iref::Vec2iref(int x2, int y2) : xValue(x2), yValue(y2) {}
Vec2iref::Vec2iref(int x2, int* y2) : xValue(x2), yValue(y2) {}
Vec2iref::Vec2iref(int* x2, int y2) : xValue(x2), yValue(y2) {}
Vec2iref::Vec2iref(int* x2, int* y2) : xValue(x2), yValue(y2) {}
Vec2iref::Vec2iref(std::variant<int, int*> x2, std::variant<int, int*> y2) : xValue(x2), yValue(y2) {};

int Vec2iref::getValue(const std::variant<int, int*>& value) {
    if (std::holds_alternative<int>(value)) {
        return std::get<int>(value);
    } else {
        return *std::get<int*>(value);
    }
}
