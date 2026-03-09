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
