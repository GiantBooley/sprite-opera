#ifndef IMAGEDATA_TPP
#define IMAGEDATA_TPP


#include <cstdint>
#include <iostream>
#include <memory>
#include <QImageReader>
template <typename T, T whiteValue>
ImageData<T, whiteValue>::ImageData() : width(0), height(0), colch(0), openGLTexture(nullptr), filePath("") {}

template <typename T, T whiteValue>
ImageData<T, whiteValue>::~ImageData() {
    if (openGLTexture) delete openGLTexture;
}

template <typename T, T whiteValue>
ImageData<T, whiteValue>::ImageData(size_t w, size_t h, size_t colc) : width(w), height(h), colch(colc), data(w * h * colc), openGLTexture(nullptr), filePath("") {}

template <typename T, T whiteValue>
ImageData<T, whiteValue>::ImageData(const char* fileName) : openGLTexture(nullptr), filePath(fileName) { // load from file
    QImageReader reader(fileName);
    reader.setDecideFormatFromContent(true);
    reader.setAutoTransform(true);
    reader.setAllocationLimit(0); // allow large images

    QImage img;

    if (!reader.read(&img)) {
        std::cout << "Failed to load image: \"" << fileName << "\": " << reader.errorString().toStdString() << std::endl;
        width = height = colch = 0;
        return;
    }
    // convert to rgba8888 if image is rgba
    if (img.format() != QImage::Format_RGBA8888_Premultiplied && colch == 4) {
        img = img.convertToFormat(QImage::Format_RGBA8888_Premultiplied);
    }


    switch (img.format()) {
    case QImage::Format_RGBA8888:
        colch = 4;
        break;
    case QImage::Format_RGBA8888_Premultiplied:
        colch = 4;
        break;
    case QImage::Format_Grayscale8:
        colch = 1;
        break;

    case QImage::Format_RGB888:
        colch = 3;
        break;

    default:
        img = img.convertToFormat(QImage::Format_RGBA8888_Premultiplied);
        colch = 4;
        break;
    }

    width  = static_cast<size_t>(img.width());
    height = static_cast<size_t>(img.height());
    const uchar* bits = img.constBits();

    const size_t totalPixels = width * height * colch;
    data.resize(totalPixels);

    const double scale = static_cast<double>(whiteValue) / 255.0;

    for (size_t i = 0; i < totalPixels; ++i)
        data[i] = static_cast<T>(bits[i] * scale);
}

template <typename T, T whiteValue>
size_t ImageData<T, whiteValue>::getWidth() const {
    return width;
}
template <typename T, T whiteValue>
size_t ImageData<T, whiteValue>::getHeight() const {
    return height;
}
template <typename T, T whiteValue>
size_t ImageData<T, whiteValue>::getChannels() const {
    return colch;
}
template <typename T, T whiteValue>
double ImageData<T, whiteValue>::sampleNormalAlpha(size_t x, size_t y) const {
    if (colch < 4) return 1.0;
    return static_cast<double>(data[(y * width + x) * colch + 3]) / static_cast<double>(whiteValue);
}
template <typename T, T whiteValue>
bool ImageData<T, whiteValue>::checkBounds(size_t x, size_t y) const {
    return x < width && y < height;
}
template <typename T, T whiteValue>
bool ImageData<T, whiteValue>::checkBounds(int x, int y) const {
    return static_cast<size_t>(x) < width && static_cast<size_t>(y) < height && x >= 0 && y >= 0;
}
template <typename T, T whiteValue>
void ImageData<T, whiteValue>::setToWhite(size_t x, size_t y) {
    T* pixel = &data[(y * width + x) * colch];
    for (size_t i = 0; i < colch; i++) {
        pixel[i] = whiteValue;
    }
}
template <typename T, T whiteValue>
void ImageData<T, whiteValue>::setToZero(size_t x, size_t y) {
    T* pixel = &data[(y * width + x) * colch];
    for (size_t i = 0; i < colch; i++) {
        pixel[i] = T();
    }
}

template <typename T, T whiteValue>
void ImageData<T, whiteValue>::bindTexture() const {
    if (!openGLTexture) {
        std::cout << "Failed to bind texture, openGLTexture is nullptr" << std::endl;
        return;
    }

    openGLTexture->bind();
}
template <typename T, T whiteValue>
void ImageData<T, whiteValue>::loadTexture() {
    if (openGLTexture) return;//delete openGLTexture;

    QImage::Format imageFormat;
    switch (colch) {
    case 1:
        imageFormat = QImage::Format_Grayscale8;
        break;
    case 3:
        imageFormat = QImage::Format_RGB888;
        break;
    case 4:
        imageFormat = QImage::Format_RGBA8888;
        break;
    default:
        std::cout << "Unsupported channel count: " << colch << std::endl;
        return;
    }
    if ((int)data.size() < width * height * colch) {
        std::cout << "ImageData::loadTexture: data buffer is empty or too small" << std::endl;
        return;
    }

    // convert image to 8 bit
    ImageData<unsigned char, 255> formatTexture = convertToType<unsigned char, 255>();
    QImage image(formatTexture.data.data(), int(width), int(height), imageFormat);

    openGLTexture = new QOpenGLTexture(image.flipped(Qt::Vertical));
    openGLTexture->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
    openGLTexture->setMagnificationFilter(QOpenGLTexture::Nearest);
    openGLTexture->setWrapMode(QOpenGLTexture::Repeat);
}
/*template <typename T, T whiteValue>
void ImageData<T, whiteValue>::saveToFile(std::string fileName) const {

}*/

template <typename T, T whiteValue>
void ImageData<T, whiteValue>::convertColorChannels(size_t newColorChannels) {
    if (colch == newColorChannels) return;

    std::vector<T> newData(width * height * newColorChannels);
    for (size_t i = 0; i < width * height; i++) {
        for (size_t j = 0; j < newColorChannels; j++) {
            if (j < colch) {
                newData[i * newColorChannels + j] = data[i * colch + j];
            } else {
                newData[i * newColorChannels + j] = (j == 3) ? whiteValue : static_cast<T>(0);
            }
        }
    }

    data = newData;
    colch = newColorChannels;
}

template <typename T, T whiteValue>
constexpr T ImageData<T, whiteValue>::getWhiteValue() {
    return whiteValue;
}

template<typename T, T whiteValue>
inline void ImageData<T, whiteValue>::getNormalizedPixelBW(size_t x, size_t y, float* gray) const {
    constexpr float m = 1.f / static_cast<float>(whiteValue);
    size_t pixel = (y * width + x) * colch;
    switch (colch) {
        case 1: case 2: *gray = static_cast<float>(data[pixel]) * m; break;// get first value of pixel
        case 3: case 4: *gray = m * (static_cast<float>(data[pixel]) * 0.299f + static_cast<float>(data[pixel + 1]) * 0.587f + static_cast<float>(data[pixel + 2]) * 0.114f);
    }
}
template<typename T, T whiteValue>
inline void ImageData<T, whiteValue>::getNormalizedPixelBWA (size_t x, size_t y, float* gray, float* alpha) const {
    constexpr float m = 1.f / static_cast<float>(whiteValue);
    size_t pixel = (y * width + x) * colch;
    switch (colch) {
        case 1: case 2: *gray = static_cast<float>(data[pixel]) * m; break;// get first value of pixel
        case 3: case 4: *gray = m * (static_cast<float>(data[pixel]) * 0.299f + static_cast<float>(data[pixel + 1]) * 0.587f + static_cast<float>(data[pixel + 2]) * 0.114f);
    }
    switch (colch) {
        case 1: case 3: *alpha = 1.f; break;
        case 2: *alpha = static_cast<float>(data[pixel + 1]) * m; break;
        case 4: *alpha = static_cast<float>(data[pixel + 3]) * m;
    }
}
template<typename T, T whiteValue>
inline void ImageData<T, whiteValue>::getNormalizedPixelRGB (size_t x, size_t y, float* red,  float* green, float* blue) const {
    constexpr float m = 1.f / static_cast<float>(whiteValue);
    size_t pixel = (y * width + x) * colch;
    float gray;
    switch (colch) {
        case 1: case 2: gray = static_cast<float>(data[pixel]) * m; *red = gray; *green = gray; *blue = gray; break;
        case 3: case 4: *red = static_cast<float>(data[pixel]) * m; *green = static_cast<float>(data[pixel + 1]) * m; *blue = static_cast<float>(data[pixel + 2]) * m;
    }
}
template<typename T, T whiteValue>
inline void ImageData<T, whiteValue>::getNormalizedPixelRGBA(size_t x, size_t y, float* red,  float* green, float* blue, float* alpha) const {
    constexpr float m = 1.f / static_cast<float>(whiteValue);
    size_t pixel = (y * width + x) * colch;
    float gray;
    switch (colch) {
        case 1: case 2: gray = static_cast<float>(data[pixel]) * m; *red = gray; *green = gray; *blue = gray; break;
        case 3: case 4: *red = static_cast<float>(data[pixel]) * m; *green = static_cast<float>(data[pixel + 1]) * m; *blue = static_cast<float>(data[pixel + 2]) * m;
    }
    switch (colch) {
        case 1: case 3: *alpha = 1.f; break;
        case 2: *alpha = static_cast<float>(data[pixel + 1]) * m; break;
        case 4: *alpha = static_cast<float>(data[pixel + 3]) * m;
    }
}

inline void setNormalizedPixelBW  (size_t x, size_t y, float gray) const override;
inline void setNormalizedPixelBWA (size_t x, size_t y, float gray, float alpha) const override;
inline void setNormalizedPixelRGB (size_t x, size_t y, float red,  float green, float blue) const override;
inline void setNormalizedPixelRGBA(size_t x, size_t y, float red,  float green, float blue, float alpha) const override;

template <typename T, T whiteValue>
template <typename newType, newType newWhiteValue>
ImageData<newType, newWhiteValue> ImageData<T, whiteValue>::convertToType() const {
    ImageData<newType, newWhiteValue> newImageData;
    newImageData.width = width;
    newImageData.height = height;
    newImageData.colch = colch;
    newImageData.data.resize(width * height * colch);
    for (size_t i = 0; i < width * height * colch; i++) {
        double intermediate = static_cast<double>(data[i]) / static_cast<double>(whiteValue);
        newImageData.data[i] = static_cast<newType>(intermediate * static_cast<double>(newWhiteValue));
    }
    return newImageData;
}

template <typename T, T whiteValue>
std::string ImageData<T, whiteValue>::getFilePath() const {
    return filePath;
}


template <typename T, T whiteValue>
std::shared_ptr<IImageData> ImageData<T, whiteValue>::convertTo(ImageType type) const {
    switch (type) {
    case FLOAT32:
        return std::make_shared<ImageData<float, 1.f>>(convertToType<float, 1.f>());
    case UINT8:
        return std::make_shared<ImageData<uint8_t, 255>>(convertToType<uint8_t, 255>());
    }
    // default uint8
    return std::make_shared<ImageData<uint8_t, 255>>(convertToType<uint8_t, 255>());
}

#endif
