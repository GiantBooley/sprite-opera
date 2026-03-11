#ifndef MATH_HPP
#define MATH_HPP

#include <variant>
#include <cstdint>
#include <vector>

// math functions
int clamp(int a, int b, int c);
float square(float x);
float inverseLerp(float x, float a, float b);
uint8_t lerp(uint8_t a, uint8_t b, float t);

// vec2
struct Vec2 {
    int x, y;
    Vec2 operator-(const Vec2& other);
    Vec2 operator+(const int& other);
};
struct Vec2f {
    float x, y;
    void operator+=(const Vec2f& other);
    Vec2f operator-(const Vec2f& other);
    Vec2f operator+(const float& other);
    Vec2f operator*(const float& other);
    Vec2f operator/(const float& other);
};
struct Vec3f {
    float x, y, z;
    void operator+=(const Vec3f& other);
    Vec3f operator-(const Vec3f& other);
    Vec3f operator+(const float& other);
    Vec3f operator*(const float& other);
    Vec3f operator/(const float& other);

    void normalize();
};
class NearestNeighbor {
public:
    int gridWidth, gridHeight;
    float minX, maxX, minY, maxY;
    std::vector<std::vector<Vec2f>> cells;
    NearestNeighbor(int width, int height, std::vector<Vec2f>& points);
    Vec2f getNearestNeighbor(Vec2f point) const;
};
class Vec2iref {
public:
    std::variant<int, int*> xValue, yValue;

    int x() const;
    int y() const;

    Vec2iref(int x2, int y2);
    Vec2iref(int x2, int* y2);
    Vec2iref(int* x2, int y2);
    Vec2iref(int* x2, int* y2);
    Vec2iref(std::variant<int, int*> x2, std::variant<int, int*> y2);
private:
    // get value from int/pointer
    static int getValue(const std::variant<int, int*>& value);
};
class Rect {
public:
    Vec2iref min, max;

    Rect(Vec2iref a, Vec2iref b) : min(a), max(b) {}

    bool intersects(const Rect& other) const {
        return !(other.min.x() >= max.x() ||
                 other.max.x() <= min.x() ||
                 other.min.y() >= max.y() ||
                 other.max.y() <= min.y());
    }
    bool contains(const Rect& other) const {
        return (min.x() <= other.min.x() && max.x() >= other.max.x() &&
                min.y() <= other.min.y() && max.y() >= other.max.y());
    }
};

#endif // MATH_HPP
