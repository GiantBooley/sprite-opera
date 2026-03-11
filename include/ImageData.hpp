#ifndef IMAGEDATA_HPP
#define IMAGEDATA_HPP

#include "IImageData.hpp"
#include <vector>

#include <QOpenGLTexture>

// 2 classes: cant convert between classes and reused code
// enum for type and multiple arrays fortypes:
// template class: only float and 8bit are needed
// only 8 bit: cant do hdr images


template <typename T, T whiteValue>
class ImageData : public IImageData {
public:
    std::vector<T> data;
    size_t width, height, colch;
    std::string filePath;

    QOpenGLTexture* openGLTexture;

    ImageData();
    ~ImageData();
    ImageData(size_t w, size_t h, size_t colc);
    ImageData(const char* fileName);

    size_t getWidth() const override;
    size_t getHeight() const override;
    size_t getChannels() const override;

    double sampleNormalAlpha(size_t x, size_t y) const override;
    bool checkBounds(size_t x, size_t y) const override;
    bool checkBounds(int x, int y) const override;
    void setToWhite(size_t x, size_t y) override;
    void setToZero(size_t x, size_t y) override;

    inline void getNormalizedPixelBW  (size_t x, size_t y, float* gray) const override;
    inline void getNormalizedPixelBWA (size_t x, size_t y, float* gray, float* alpha) const override;
    inline void getNormalizedPixelRGB (size_t x, size_t y, float* red,  float* green, float* blue) const override;
    inline void getNormalizedPixelRGBA(size_t x, size_t y, float* red,  float* green, float* blue, float* alpha) const override;
    inline void getNormalizedPixelA   (size_t x, size_t y, float* alpha) const override;

    inline void setNormalizedPixelBW  (size_t x, size_t y, float gray) override;
    inline void setNormalizedPixelBWA (size_t x, size_t y, float gray, float alpha) override;
    inline void setNormalizedPixelRGB (size_t x, size_t y, float red,  float green, float blue) override;
    inline void setNormalizedPixelRGBA(size_t x, size_t y, float red,  float green, float blue, float alpha) override;
    inline void setNormalizedPixelA   (size_t x, size_t y, float alpha) override;

    void bindTexture() const override;
    void loadTexture() override;

    //void saveToFile(std::string fileName) const override;
    void convertColorChannels(size_t newColorChannels) override;

    static constexpr T getWhiteValue();

    template <typename newType, newType newWhiteValue>
    ImageData<newType, newWhiteValue> convertToType() const;

    std::shared_ptr<IImageData> convertTo(ImageType type) const override;

    std::string getFilePath() const override;
};
#include "../ImageData.tpp"
#endif
