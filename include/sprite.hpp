#ifndef SPRITE_H
#define SPRITE_H
#include <qabstractitemmodel.h>
#include <qobject.h>
#include <vector>
#include <memory>
#include "IImageData.hpp"
#include "ImageData.hpp"
#include "binarymask.hpp"
#include "math.hpp"

class UnityAsset {
public:
    std::string assetPath;
};
class ColorGrade {
public:
    float multiplyR, multiplyG, multiplyB, exponentR, exponentG, exponentB, saturation;
    ColorGrade(float rm, float gm, float bm, float re, float ge, float be, float s);
    ColorGrade();
    void applyToColor(float& r, float& g, float& b) const;
    void undoToColor(float& r, float& g, float& b) const;
};
// definition of sprite: region of an image
class Sprite {
public:
    std::shared_ptr<IImageData> image;
    std::shared_ptr<IImageData> normalMap;
    std::shared_ptr<AbstractBinaryMask> region;
    ColorGrade colorGrade;

    Sprite(std::shared_ptr<IImageData> img);
    Sprite(std::shared_ptr<IImageData> img, std::shared_ptr<AbstractBinaryMask> regio);

    std::shared_ptr<IImageData> rasterizeTo(ImageType type) const;

    static bool sortSizeDescending(const Sprite& a, const Sprite& b);
};
class SpritePositionedInRaster {
public:
    Sprite sprite;
    Vec2 position;
    uint8_t rotation;

    SpritePositionedInRaster(Sprite sprit, Vec2 pos, uint8_t rot);

    int getWidth() const;
    int getHeight() const;

    static bool sortSizeDescending(const SpritePositionedInRaster& a, const SpritePositionedInRaster& b);
};

class VirtualSpritesheet : public QAbstractItemModel {
    Q_OBJECT
public slots:
    void repack();
    void rasterizeToFile();
    void rasterizeSpritesToFolder();
public:
    std::vector<SpritePositionedInRaster> sprites;

    void rectPack(std::vector<Sprite> images, std::vector<Rect>* freeRects, int spriteMargin);
    void separate(std::shared_ptr<IImageData> spritesheet);
    void trimAllSprites();

    size_t getTotalWidth() const;
    size_t getTotalHeight() const;


    std::shared_ptr<IImageData> rasterizeTo(ImageType type) const;

    VirtualSpritesheet();

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
private:
};
#endif // SPRITE_H
