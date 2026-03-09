#ifndef BINARYMASK_HPP
#define BINARYMASK_HPP
#include <cstdint>
#include <vector>
#include <math.hpp>
class AbstractBinaryMask {
public:
    virtual bool sample(int x, int y) const = 0;
    virtual Vec2 getMin() const = 0;
    virtual Vec2 getMax() const = 0;
    virtual void cropToContent() = 0;
    virtual void makeWidthAndHeightDivisibleBy(int x) = 0;
};

class BinaryMaskData {
public:
    size_t width, height;

    std::vector<uint8_t> data;
    BinaryMaskData(size_t width2, size_t height2);

    bool sample(size_t x, size_t y) const;
    void set(size_t x, size_t y, bool value);
    void clearAll();

    void crop(Vec2 newMin, Vec2 newMax);
};
class BinaryRasterMask : public AbstractBinaryMask {
public:
    Vec2 min, max;

    BinaryMaskData data;
    BinaryRasterMask(Vec2 min2, Vec2 max2);

    void set(int x, int y, bool value);

    bool sample(int x, int y) const override;
    Vec2 getMin() const override;
    Vec2 getMax() const override;
    void cropToContent() override;
    void makeWidthAndHeightDivisibleBy(int x) override;

private:
    size_t getWidth() const;
    size_t getHeight() const;
};
class BinaryVectorMask : public AbstractBinaryMask {
public:
    std::vector<std::vector<Vec2f>> paths;
    Vec2f min, max;
    BinaryVectorMask(Vec2f min2, Vec2f max2);

    bool sample(int x, int y) const override;
    Vec2 getMin() const override;
    Vec2 getMax() const override;
    void cropToContent() override;
    void makeWidthAndHeightDivisibleBy(int x) override;
};

#endif // BINARYMASK_HPP
