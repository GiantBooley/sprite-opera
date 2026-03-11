//spritesheet separator v8 by giantbooley

#include <iomanip>
#include <sstream>
#include <iostream>
#include <vector>
#include <algorithm>
#include <QFileDialog>
#include "ImageData.hpp"
#include "sprite.hpp"
#include "math.hpp"
#include "stringcreator.hpp"
#include "binarymask.hpp"
#include "normalmapgenerator.hpp"

// to fix: square spritesheet joiner, add outlines checkbox
// maybe:: add operation system, each thing outputs an operation summary, show progress bar

ColorGrade::ColorGrade(float rm, float gm, float bm, float re, float ge, float be, float s) : multiplyR(rm), multiplyG(gm), multiplyB(bm), exponentR(re), exponentG(ge), exponentB(be), saturation(s) {}
ColorGrade::ColorGrade() : multiplyR(1.f), multiplyG(1.f), multiplyB(1.f), exponentR(1.f), exponentG(1.f), exponentB(1.f), saturation(1.f) {}
void ColorGrade::applyToColor(float& r, float& g, float& b) const {
    r = std::pow(r, exponentR) * multiplyR;
    g = std::pow(r, exponentG) * multiplyG;
    b = std::pow(r, exponentB) * multiplyB;
}
void ColorGrade::undoToColor(float& r, float& g, float& b) const {
    r = std::pow(r / multiplyR, 1.f / exponentR);
    g = std::pow(r / multiplyG, 1.f / exponentG);
    b = std::pow(r / multiplyB, 1.f / exponentB);
}


Sprite::Sprite(std::shared_ptr<IImageData> img) :
    image(img),
    region(std::make_shared<BinaryRasterMask>(Vec2(0, 0), Vec2(static_cast<int>(img->getWidth()), static_cast<int>(img->getHeight())))) {}
Sprite::Sprite(std::shared_ptr<IImageData> img, std::shared_ptr<AbstractBinaryMask> regio) :
    image(img),
    region(regio) {
    normalMap = std::make_shared<ImageData<float, 1.f>>(NormalMapGenerator::generate(img));
}
bool Sprite::sortSizeDescending(const Sprite& a, const Sprite& b) {
    return (a.region->getMax().x - a.region->getMin().x) * (a.region->getMax().y - a.region->getMin().y) > (b.region->getMax().x - b.region->getMin().x) * (b.region->getMax().y - b.region->getMin().y);
}
std::shared_ptr<IImageData> Sprite::rasterizeTo(ImageType type) const {
    size_t width = region->getMax().x - region->getMin().x;
    size_t height = region->getMax().y - region->getMin().y;
    // create result array full of 0 based on type
    std::shared_ptr<IImageData> result;
    switch (type) {
    case FLOAT32:{
        result = std::make_shared<ImageData<float, 1.f>>(width, height, 4);
        std::shared_ptr<ImageData<float, 1.f>> resultImage = std::dynamic_pointer_cast<ImageData<float, 1.f>>(result);
        std::fill(resultImage->data.begin(), resultImage->data.end(), 0);

        std::shared_ptr<ImageData<float, 1.f>> spriteImage = std::static_pointer_cast<ImageData<float, 1.f>>(image);

        for (int y = region->getMin().y; y < region->getMax().y; y++) {
            for (int x = region->getMin().x; x < region->getMax().x; x++) {
                size_t resultPixelIndex = ((y - region->getMin().y) * resultImage->getWidth() + x - region->getMin().x) * resultImage->getChannels();
                if (x < 0 || y < 0 || x >= spriteImage->getWidth() || y >= spriteImage->getHeight()) { // overflow image then draw empty
                    resultImage->data[resultPixelIndex]     = 0.f;
                    resultImage->data[resultPixelIndex + 1] = 0.f;
                    resultImage->data[resultPixelIndex + 2] = 0.f;
                    resultImage->data[resultPixelIndex + 3] = 0.f;
                    continue;
                }
                size_t spritePixelIndex = (y * image->getWidth() + x) * image->getChannels();

                float spriteAlpha = spriteImage->data[spritePixelIndex + 3];
                resultImage->data[resultPixelIndex]     = spriteImage->data[spritePixelIndex    ];
                resultImage->data[resultPixelIndex + 1] = spriteImage->data[spritePixelIndex + 1];
                resultImage->data[resultPixelIndex + 2] = spriteImage->data[spritePixelIndex + 2];
                resultImage->data[resultPixelIndex + 3] = spriteAlpha;
            }
        }
        break;
    }
    case UINT8:{
        result = std::make_shared<ImageData<uint8_t, 255>>(width, height, 4);
        std::shared_ptr<ImageData<uint8_t, 255>> resultImage = std::dynamic_pointer_cast<ImageData<uint8_t, 255>>(result);
        std::fill(resultImage->data.begin(), resultImage->data.end(), 0);

        std::shared_ptr<ImageData<uint8_t, 255>> spriteImage = std::static_pointer_cast<ImageData<uint8_t, 255>>(image);

        for (int y = region->getMin().y; y < region->getMax().y; y++) {
            for (int x = region->getMin().x; x < region->getMax().x; x++) {
                size_t resultPixelIndex = ((y - region->getMin().y) * resultImage->getWidth() + x - region->getMin().x) * resultImage->getChannels();
                if (x < 0 || y < 0 || x >= spriteImage->getWidth() || y >= spriteImage->getHeight()) { // overflow image then draw empty
                    resultImage->data[resultPixelIndex]     = 0;
                    resultImage->data[resultPixelIndex + 1] = 0;
                    resultImage->data[resultPixelIndex + 2] = 0;
                    resultImage->data[resultPixelIndex + 3] = 0;
                    continue;
                }
                size_t spritePixelIndex = (y * image->getWidth() + x) * image->getChannels();

                uint8_t spriteAlpha = spriteImage->data[spritePixelIndex + 3];
                resultImage->data[resultPixelIndex]     = spriteImage->data[spritePixelIndex    ];
                resultImage->data[resultPixelIndex + 1] = spriteImage->data[spritePixelIndex + 1];
                resultImage->data[resultPixelIndex + 2] = spriteImage->data[spritePixelIndex + 2];
                resultImage->data[resultPixelIndex + 3] = spriteAlpha;
            }
        }
        break;
    }
    }

    return result;
}


SpritePositionedInRaster::SpritePositionedInRaster(Sprite sprit, Vec2 pos, uint8_t rot) : sprite(sprit), position(pos), rotation(rot) {
}
int SpritePositionedInRaster::getWidth() const {
    return (rotation % 2 == 0) ? (sprite.region->getMax().x - sprite.region->getMin().x) : (sprite.region->getMax().y - sprite.region->getMin().y);
}
int SpritePositionedInRaster::getHeight() const {
    return (rotation % 2 == 0) ? (sprite.region->getMax().y - sprite.region->getMin().y) : (sprite.region->getMax().x - sprite.region->getMin().x);
}
bool SpritePositionedInRaster::sortSizeDescending(const SpritePositionedInRaster& a, const SpritePositionedInRaster& b) {
    return Sprite::sortSizeDescending(a.sprite, b.sprite);
}

// slots
void VirtualSpritesheet::repack() {
    std::vector<Sprite> spritesToPack;
    spritesToPack.reserve(sprites.size());
    for (size_t i = 0; i < sprites.size(); i++) {
        spritesToPack.push_back(sprites[i].sprite);
    }
    sprites.clear();
    rectPack(spritesToPack, nullptr, 1);
}
void VirtualSpritesheet::rasterizeToFile() {
    QString fileName = QFileDialog::getSaveFileName(nullptr,
        tr("Save Spritesheet Image"),
        "",
        tr("Images (*.png);;All Files (*)"));
    if (!fileName.isEmpty()) {
        std::shared_ptr<IImageData> raster = rasterizeTo(UINT8);
        std::shared_ptr<ImageData<uint8_t, 255>> raster8BitImage = std::static_pointer_cast<ImageData<uint8_t, 255>>(raster);

        QImage image(raster8BitImage->data.data(), raster->getWidth(), raster->getHeight(), QImage::Format_RGBA8888);
        image.save(fileName);
    }

}
void VirtualSpritesheet::rasterizeSpritesToFolder() {
    QString folderPath = QFileDialog::getExistingDirectory(
        nullptr,
        tr("Choose folder"),
        QDir::homePath(),
        QFileDialog::ShowDirsOnly
        );
    if (folderPath.isEmpty()) return;

    for (size_t i = 0; i < sprites.size(); i++) {
        std::cout << "Saving image " << (i + 1) << "/" << sprites.size() << std::endl;
        std::shared_ptr<IImageData> raster = sprites[i].sprite.rasterizeTo(UINT8);
        std::shared_ptr<ImageData<uint8_t, 255>> raster8BitImage = std::static_pointer_cast<ImageData<uint8_t, 255>>(raster);

        QImage image(raster8BitImage->data.data(), raster->getWidth(), raster->getHeight(), QImage::Format_RGBA8888_Premultiplied);
        image.save(QDir::cleanPath(folderPath + QDir::separator() + QString("Cactus%1.png").arg(i + 1)));
    }

}

void VirtualSpritesheet::separate(std::shared_ptr<IImageData> spritesheet) {
    std::cout << "separating spritesheet size: " << spritesheet->getWidth() << "x" << spritesheet->getHeight() << std::endl;
    // ===== settings ======
    const double alphaThreshold = 0.0; // pixels with alpha above this number will be kept (0-1), 0=keep everything above 0, 1=delete everything
    const size_t minArea = 5 * 5; // minimum number of pixels for island to be valid

    // ===== summary variables
    size_t skippedIslands = 0;

    size_t width = spritesheet->getWidth();
    size_t height = spritesheet->getHeight();

    // return nothing if image has no alpha
    if (spritesheet->getChannels() < 4) {
        sprites.push_back({{spritesheet}, {0, 0}, 0});
        return;
    }

    // ===== create alpha threshold binary map =====
    BinaryRasterMask alphaMap{{0, 0}, {(int)width, (int)height}};
    for (size_t y = 0; y < height; y++) {
        for (size_t x = 0; x < width; x++) {
            if (spritesheet->sampleNormalAlpha(x, y) > alphaThreshold) {
                alphaMap.set(x, y, true);
            }
        }
    }


    BinaryRasterMask visited{{0, 0}, {(int)spritesheet->getWidth(), (int)spritesheet->getHeight()}}; // either: set visited to true, or delete pixel from alpha map
    // ===== loop over every pixel and add islands when they are encountered =====

    const Vec2 checkOffsets[4] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
    for (size_t y = 0; y < height; y++) {
        for (size_t x = 0; x < width; x++) {
            if (alphaMap.sample(x, y)) { // check if has alpha at x y (start island)
                Vec2 islandMin{static_cast<int>(x), static_cast<int>(y)};
                Vec2 islandMax{static_cast<int>(x), static_cast<int>(y)};

                visited.set(x, y, true);
                std::vector<Vec2> floodPixelList = {{static_cast<int>(x), static_cast<int>(y)}}; // pixels on the edge of flood fill
                size_t area = 1;

                while (!floodPixelList.empty()) { // expand flood by 1 until flooded everything
                    Vec2 floodPos = floodPixelList.back(); // get last pos in list, then remove it (depth first search)
                    floodPixelList.pop_back();

                    for (Vec2 offset : checkOffsets) {
                        int newX = floodPos.x + offset.x;
                        int newY = floodPos.y + offset.y;
                        // check if flood pixel x isnt 0 and the pixel to the left of it has alpha and the pixel to the left of it hasnt been checked
                        if (spritesheet->checkBounds(newX, newY) && // check bounds
                            alphaMap.sample(newX, newY) && // check if new position has alpha
                            !visited.sample(newX, newY) // check if the pixel hasnt been visited yet
                        ) {
                            floodPixelList.push_back({newX, newY});
                            visited.set(newX, newY, true);
                            area++; // try moving outside loop
                        }
                    }

                    // expand sprite bounds
                    islandMin.x = std::min(islandMin.x, floodPos.x);
                    islandMax.x = std::max(islandMax.x, floodPos.x);
                    islandMin.y = std::min(islandMin.y, floodPos.y);
                    islandMax.y = std::max(islandMax.y, floodPos.y);
                }
                if (area < minArea) {
                    skippedIslands++;
                    continue;
                }



                // remove island and create island mask for sprite
                std::shared_ptr<BinaryRasterMask> islandMask = std::make_shared<BinaryRasterMask>(Vec2(islandMin.x, islandMin.y), Vec2(islandMax.x + 1, islandMax.y + 1));
                for (int ya = islandMin.y; ya <= islandMax.y; ya++) {
                    for (int xa = islandMin.x; xa <= islandMax.x; xa++) {
                        if (visited.sample(xa, ya)) {
                            alphaMap.set(xa, ya, false);
                            islandMask->set(xa - islandMin.x, ya - islandMin.y, true);
                            visited.set(xa, ya, false);
                        }
                    }
                }

                // add
                Sprite sprite{spritesheet, islandMask};
                SpritePositionedInRaster positionedSprite{sprite, islandMin, 0};
                sprites.push_back(positionedSprite);
            }
        }
    }
    std::cout << "[INFO] Spritesheet separate summary" << std::endl;
    std::cout << " -skipped islands: " << skippedIslands << std::endl;
}
void VirtualSpritesheet::rectPack(std::vector<Sprite> spritesToPack, std::vector<Rect>* freeRects, int spriteMargin = 100) {
    // ===== create sprites array =====
    std::sort(spritesToPack.begin(), spritesToPack.end(), Sprite::sortSizeDescending);

    // ===== init 1x1 rect =====
    std::cout << "[INFO] Starting maxrects algorithm..." << std::endl;
    std::vector<Rect> maxRects;

    // // start size at 1x1. add sprite size to spritesheet size before adding. after adding, reduce size to max size
    Vec2 largestMaxPos = {0, 0};
    maxRects.push_back({{0, 0}, {&largestMaxPos.x, &largestMaxPos.y}});
    int skippedSprites = 0;

    // ===== add sprites to spritesheet =====
    for (size_t i = 0; i < spritesToPack.size(); i++) {
        if (i % 10 == 0 || i == spritesToPack.size() - 1) { // print
            float percent = static_cast<float>(i) / static_cast<float>(spritesToPack.size());
            std::cout << "[INFO] " << generateProgressBar(percent, 50) << "[" << (i + 1) << "/" << spritesToPack.size() << "] Adding sprite to sheet, rects: " << maxRects.size();
            if (i != spritesToPack.size() - 1) std::cout << "\r";
            std::cout.flush();
        }
        int totalSpriteWidth = (spritesToPack[i].region->getMax().x - spritesToPack[i].region->getMin().x) + spriteMargin * 2;
        int totalSpriteHeight = (spritesToPack[i].region->getMax().y - spritesToPack[i].region->getMin().y) + spriteMargin * 2;

        // expanding logic: first check if image can fit normally using heuristic, if it cant then set size to infinity and choose whichever that increases area the least amount

        // ===== choose best rect to place image in =====
        int index = -1;

        int bestAreaFit = 0;
        int bestShortSideFit = 0;
        for (size_t j = 0; j < maxRects.size(); j++) {
            // get size of max rect
            int rectWidth = maxRects[j].max.x() - maxRects[j].min.x() + 1;
            int rectHeight = maxRects[j].max.y() - maxRects[j].min.y() + 1;

            if (rectWidth >= totalSpriteWidth && rectHeight >= totalSpriteHeight) {
                int areaFit = (rectWidth * rectHeight) - (totalSpriteWidth * totalSpriteHeight); // remaining area
                int shortSideFit = std::min(rectWidth - totalSpriteWidth, rectHeight - totalSpriteHeight);

                if (index == -1 || areaFit < bestAreaFit || (areaFit == bestAreaFit && shortSideFit < bestShortSideFit)) {
                    bestAreaFit = areaFit;
                    bestShortSideFit = shortSideFit;
                    index = j;
                }
            }
        }

        if (index == -1) {
            // expand spritesheet. choose location that expands the spritesheet the least amount
            // potential optimization: get expandableIndices in first loop
            // potential optimization: add rotation
            // potential optimization: use least spritesheet area heuristic when expanding, expand to the shorter side

            // init best variables
            long long lowestSpritesheetArea = -1ll;
            double highestAspectRatioScore = 0.;
            int bestShortSideFit = 0;
            Vec2 bestMaxPos;

            for (size_t j = 0; j < maxRects.size(); j++) {
                bool isXEdge = (std::holds_alternative<int*>(maxRects[j].max.xValue) && std::get<int*>(maxRects[j].max.xValue) == &largestMaxPos.x);
                bool isYEdge = (std::holds_alternative<int*>(maxRects[j].max.yValue) && std::get<int*>(maxRects[j].max.yValue) == &largestMaxPos.y);

                if (!isXEdge && !isYEdge) {
                    continue;
                }
                // get size of max rect
                int rectWidth = maxRects[j].max.x() - maxRects[j].min.x() + 1;
                int rectHeight = maxRects[j].max.y() - maxRects[j].min.y() + 1;

                if ((isXEdge || rectWidth >= totalSpriteWidth) && (isYEdge || rectHeight >= totalSpriteHeight)) { // if sprite can fit
                    Rect spriteRect{maxRects[j].min, {maxRects[j].min.x() + totalSpriteWidth - 1, maxRects[j].min.y() + totalSpriteHeight - 1}};

                    Vec2 newSpritesheetMax = {
                        std::max(spriteRect.max.x(), largestMaxPos.x),
                        std::max(spriteRect.max.y(), largestMaxPos.y)
                    };

                    long long newSpritesheetWidth = newSpritesheetMax.x + 1ll;
                    long long newSpritesheetHeight = newSpritesheetMax.y + 1ll;
                    long long newSpritesheetArea = newSpritesheetWidth * newSpritesheetHeight;
                    double aspectRatioScore = std::min(static_cast<double>(newSpritesheetWidth) / static_cast<double>(newSpritesheetHeight),
                                                       static_cast<double>(newSpritesheetHeight) / static_cast<double>(newSpritesheetWidth));

                    // get short side fit. only set if the edge isnt infinite
                    int shortSideFit = bestShortSideFit;
                    if (!isXEdge) shortSideFit = rectHeight - totalSpriteHeight;
                    if (!isYEdge) shortSideFit = rectWidth - totalSpriteWidth;

                    bool best = false;
                    if (index == -1) best = true;
                    else if (aspectRatioScore > highestAspectRatioScore) best = true;
                    else if (aspectRatioScore == highestAspectRatioScore) {
                        if (newSpritesheetArea < lowestSpritesheetArea) best = true;
                        else if (newSpritesheetArea == lowestSpritesheetArea) {
                            if (shortSideFit < bestShortSideFit) best = true;
                        }
                    }
                    if (best) {
                        // new best
                        lowestSpritesheetArea = newSpritesheetArea;
                        highestAspectRatioScore = aspectRatioScore;
                        bestShortSideFit = shortSideFit;
                        bestMaxPos = newSpritesheetMax;
                        index = j;
                    }
                }
            }
            // set max pos
            largestMaxPos.x = bestMaxPos.x;
            largestMaxPos.y = bestMaxPos.y;
            /*std::cout << "[WARNING] Sprite skipped, unable to fit" << std::endl;
            sprites.erase(sprites.begin() + i);
            i--;
            skippedSprites++;
            continue;*/
        }

        Vec2 newPosition = {maxRects[index].min.x(), maxRects[index].min.y()};

        // split maxrects that are intersecting with newly placed rect
        Rect newSpriteRect = {
            {newPosition.x, newPosition.y},
            {newPosition.x + totalSpriteWidth - 1, newPosition.y + totalSpriteHeight - 1}
        };
        int newRectCount = 0;
        int originalMaxRectsSize = maxRects.size();
        for (size_t j = 0; j < originalMaxRectsSize; j++) {
            if (!maxRects[j].intersects(newSpriteRect)) { // doesnt intersect with sprite
                continue;
            }
            bool isXEdge = (std::holds_alternative<int*>(maxRects[j].max.xValue) && std::get<int*>(maxRects[j].max.xValue) == &largestMaxPos.x);
            bool isYEdge = (std::holds_alternative<int*>(maxRects[j].max.yValue) && std::get<int*>(maxRects[j].max.yValue) == &largestMaxPos.y);

            std::vector<Rect> splitRects;
            // if rect goes left of spriterect
            if (maxRects[j].min.x() < newSpriteRect.min.x()) {
                splitRects.push_back({{maxRects[j].min.xValue, maxRects[j].min.yValue}, {newSpriteRect.min.x() - 1, maxRects[j].max.yValue}});
            }
            // if rect goes down of spriterect
            if (maxRects[j].min.y() < newSpriteRect.min.y()) {
                splitRects.push_back({{maxRects[j].min.xValue, maxRects[j].min.yValue}, {maxRects[j].max.xValue, newSpriteRect.min.y() - 1}});
            }
            // if rect goes right of spriterect
            if (isXEdge || maxRects[j].max.x() > newSpriteRect.max.x()) {
                splitRects.push_back({{newSpriteRect.max.x() + 1, maxRects[j].min.yValue}, {maxRects[j].max.xValue, maxRects[j].max.yValue}});
            }
            // if rect goes up of spriterect
            if (isYEdge || maxRects[j].max.y() > newSpriteRect.max.y()) {
                splitRects.push_back({{maxRects[j].min.xValue, newSpriteRect.max.y() + 1}, {maxRects[j].max.xValue, maxRects[j].max.yValue}});
            }

            // delete maxrect that was just split into multiple smaller maxrects
            maxRects.erase(maxRects.begin() + j);
            j--;
            originalMaxRectsSize--;
            maxRects.insert(maxRects.end(), splitRects.begin(), splitRects.end());
            newRectCount += (int)splitRects.size();
        }
        // delete all maxrects that were just created that are inside another
        for (size_t j = maxRects.size() - newRectCount; j < maxRects.size(); j++) { // for each newly added rect
            for (size_t k = 0; k < maxRects.size(); k++) { // delete rect if its inside another
                if (j == k) continue;
                if (maxRects[k].contains(maxRects[j])) {
                    maxRects.erase(maxRects.begin() + j);
                    j--; // only reduce j
                    //k--;
                    break;
                }
            }
        }

        sprites.push_back({spritesToPack[i], newPosition + spriteMargin, 0});
    }
    std::cout << std::endl;

    if (freeRects) {
        for (size_t i = 0; i < maxRects.size(); i++) freeRects->push_back({{maxRects[i].min.x(), maxRects[i].min.y()}, {maxRects[i].max.x(), maxRects[i].max.y()}});
    }

    // calculate wasted space
    long long totalSpriteArea = 0;
    for (size_t i = 0; i < spritesToPack.size(); i++) {
        long long width = static_cast<long long>(spritesToPack[i].region->getMax().x - spritesToPack[i].region->getMin().x);
        long long height = static_cast<long long>(spritesToPack[i].region->getMax().y - spritesToPack[i].region->getMin().y);
        totalSpriteArea += width * height;
    }


    std::cout << "[INFO] Finished adding sprites" << std::endl;
    std::cout << "- spritesheet size: " << (largestMaxPos.x + 1) << "x" << (largestMaxPos.y + 1) << std::endl;
    long long optimalWidth = static_cast<long long>(std::ceil(std::sqrt(static_cast<double>(totalSpriteArea))));
    std::cout << "- optimal size: " << optimalWidth << "x" << optimalWidth << std::endl;
    std::stringstream ss;
    double percent = static_cast<double>(totalSpriteArea) / static_cast<double>((static_cast<long long>(largestMaxPos.x) + 1ll) * (static_cast<long long>(largestMaxPos.y) + 1ll)) * 100.f;
    ss << std::fixed << std::setprecision(2) << percent << "%";
    std::cout << "- packing score: " << ss.str() << std::endl;
    std::cout << "- wasted pixels: " << ((static_cast<long long>(largestMaxPos.x) + 1ll) * (static_cast<long long>(largestMaxPos.y) + 1ll) - totalSpriteArea) << std::endl;
    std::cout << "- skipped sprites: " << skippedSprites << std::endl;
}

size_t VirtualSpritesheet::getTotalWidth() const {
    size_t maxWidth = 0;
    for (const SpritePositionedInRaster& sprite : sprites) {
        if (sprite.position.x + sprite.getWidth() > maxWidth) {
            maxWidth = sprite.position.x + sprite.getWidth();
        }
    }
    return maxWidth;
}
size_t VirtualSpritesheet::getTotalHeight() const {
    size_t maxHeight = 0;
    for (const SpritePositionedInRaster& sprite : sprites) {
        if (sprite.position.y + sprite.getHeight() > maxHeight) {
            maxHeight = sprite.position.y + sprite.getHeight();
        }
    }
    return maxHeight;
}

std::shared_ptr<IImageData> VirtualSpritesheet::rasterizeTo(ImageType type) const {
    size_t width = getTotalWidth();
    size_t height = getTotalHeight();
    // create result array full of 0 based on type
    std::shared_ptr<IImageData> result;
    switch (type) {
        case FLOAT32:{
            result = std::make_shared<ImageData<float, 1.f>>(width, height, 4);
            std::shared_ptr<ImageData<float, 1.f>> resultImage = std::dynamic_pointer_cast<ImageData<float, 1.f>>(result);
            std::fill(resultImage->data.begin(), resultImage->data.end(), 0);

            int i = 0;
            for (const SpritePositionedInRaster& sprite : sprites) {
                std::cout << "rasterizing image : " << i << std::endl;
                i++;
                std::shared_ptr<IImageData> image = sprite.sprite.image->convertTo(type);
                std::shared_ptr<ImageData<float, 1.f>> spriteImage = std::static_pointer_cast<ImageData<float, 1.f>>(image);

                for (size_t y = 0; y < sprite.getHeight(); y++) {
                    for (size_t x = 0; x < sprite.getWidth(); x++) {
                        size_t spritePixelIndex = (y * image->getWidth() + x) * image->getChannels();
                        size_t sheetPixelIndex = ((y + sprite.position.y) * resultImage->getWidth() + x + sprite.position.x) * resultImage->getChannels();

                        float srcA = spriteImage->data[spritePixelIndex + 3];
                        if (srcA <= 0.f) continue;
                        float srcR = spriteImage->data[spritePixelIndex    ];
                        float srcG = spriteImage->data[spritePixelIndex + 1];
                        float srcB = spriteImage->data[spritePixelIndex + 2];
                        sprite.sprite.colorGrade.applyToColor(srcR, srcG, srcB);
                        float dstR = resultImage->data[sheetPixelIndex    ];
                        float dstG = resultImage->data[sheetPixelIndex + 1];
                        float dstB = resultImage->data[sheetPixelIndex + 2];
                        float dstA = resultImage->data[sheetPixelIndex + 3];
                        float outA = srcA + dstA * (1.0f - srcA);
                        float outR = (srcR * srcA + dstR * dstA * (1.0f - srcA)) / outA;
                        float outG = (srcG * srcA + dstG * dstA * (1.0f - srcA)) / outA;
                        float outB = (srcB * srcA + dstB * dstA * (1.0f - srcA)) / outA;
                        resultImage->data[sheetPixelIndex]     = outR;
                        resultImage->data[sheetPixelIndex + 1] = outG;
                        resultImage->data[sheetPixelIndex + 2] = outB;
                        resultImage->data[sheetPixelIndex + 3] = outA;
                    }
                }
            }
            break;
        }
        case UINT8:{
            result = std::make_shared<ImageData<uint8_t, 255>>(width, height, 4);
            std::shared_ptr<ImageData<uint8_t, 255>> resultImage = std::dynamic_pointer_cast<ImageData<uint8_t, 255>>(result);
            std::fill(resultImage->data.begin(), resultImage->data.end(), 0);

            int i = 0;
            for (const SpritePositionedInRaster& sprite : sprites) {
                std::cout << "rasterizing image : " << i << std::endl;
                i++;
                std::shared_ptr<IImageData> image = sprite.sprite.image->convertTo(type);
                std::shared_ptr<ImageData<uint8_t, 255>> spriteImage = std::static_pointer_cast<ImageData<uint8_t, 255>>(image);

                for (size_t y = 0; y < sprite.getHeight(); y++) {
                    for (size_t x = 0; x < sprite.getWidth(); x++) {
                        size_t srcx = sprite.sprite.region->getMin().x + x;
                        size_t srcy = sprite.sprite.region->getMin().y + y;

                        size_t spritePixelIndex = (srcy * image->getWidth() + srcx) * image->getChannels();
                        size_t sheetPixelIndex = ((y + sprite.position.y) * resultImage->getWidth() + x + sprite.position.x) * resultImage->getChannels();

                        float srcA = static_cast<float>(spriteImage->data[spritePixelIndex + 3]) / 255.f;
                        if (srcA <= 0.f) continue;
                        float srcR = static_cast<float>(spriteImage->data[spritePixelIndex    ]) / 255.f;
                        float srcG = static_cast<float>(spriteImage->data[spritePixelIndex + 1]) / 255.f;
                        float srcB = static_cast<float>(spriteImage->data[spritePixelIndex + 2]) / 255.f;
                        sprite.sprite.colorGrade.applyToColor(srcR, srcG, srcB);
                        float dstR = static_cast<float>(resultImage->data[sheetPixelIndex]) / 255.f;
                        float dstG = static_cast<float>(resultImage->data[sheetPixelIndex + 1]) / 255.f;
                        float dstB = static_cast<float>(resultImage->data[sheetPixelIndex + 2]) / 255.f;
                        float dstA = static_cast<float>(resultImage->data[sheetPixelIndex + 3]) / 255.f;
                        float outA = srcA + dstA * (1.0f - srcA);
                        float outR = (srcR * srcA + dstR * dstA * (1.0f - srcA)) / outA;
                        float outG = (srcG * srcA + dstG * dstA * (1.0f - srcA)) / outA;
                        float outB = (srcB * srcA + dstB * dstA * (1.0f - srcA)) / outA;
                        resultImage->data[sheetPixelIndex]     = static_cast<uint8_t>(std::clamp(outR * 255.f, 0.f, 255.f));
                        resultImage->data[sheetPixelIndex + 1] = static_cast<uint8_t>(std::clamp(outG * 255.f, 0.f, 255.f));
                        resultImage->data[sheetPixelIndex + 2] = static_cast<uint8_t>(std::clamp(outB * 255.f, 0.f, 255.f));
                        resultImage->data[sheetPixelIndex + 3] = static_cast<uint8_t>(std::clamp(outA * 255.f, 0.f, 255.f));
                    }
                }
            }
            break;
        }
    }

    return result;
}

void VirtualSpritesheet::trimAllSprites() {
    for (size_t i = 0; i < sprites.size(); i++) {
        sprites[i].sprite.region->cropToContent();
        sprites[i].sprite.region->makeWidthAndHeightDivisibleBy(4);
    }
}

VirtualSpritesheet::VirtualSpritesheet() {
    sprites = {};
}
int VirtualSpritesheet::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;
    return static_cast<int>(sprites.size());
}
int VirtualSpritesheet::columnCount(const QModelIndex& parent) const {
    return 2;
}
QModelIndex VirtualSpritesheet::index(int row, int column, const QModelIndex& parent) const {
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    if (!parent.isValid()) {
        return createIndex(row, column, nullptr);
    }

    return QModelIndex();
}
QModelIndex VirtualSpritesheet::parent(const QModelIndex &index) const {
    return QModelIndex();
}
QVariant VirtualSpritesheet::data(const QModelIndex &index, int role) const {
    if (!index.isValid()) return QVariant();

    if (role != Qt::DisplayRole) return QVariant();

    const SpritePositionedInRaster& spriteItem = sprites.at(index.row());

    switch (index.column()) {
    case 0:
        return QString::fromStdString(spriteItem.sprite.image->getFilePath());
    case 1:
        return QString("(%1, %2)").arg(spriteItem.getWidth()).arg(spriteItem.getHeight());
    default:
        return QVariant();
    }
}
int oldmain(int argc, char* argv[]) {
    /*} else if (argc == 4 && std::string(argv[1]) == "downscale") {
		std::string inputFolderName = argv[2];
		std::string outputFolderName = argv[3];

		std::string inputPathName = "./" + inputFolderName;
		std::filesystem::path inputPath{inputPathName.c_str()};
		if (!std::filesystem::is_directory(inputPath)) {
			std::cout << "error: input path doesnt exist" << std::endl;
			return 1;
		}

		std::string outputPathName = "./" + outputFolderName;
		std::filesystem::path outputPath{outputFolderName.c_str()};
		if (!std::filesystem::is_directory(outputPath)) {
			std::filesystem::create_directories(outputPathName.c_str());
		}

		std::ofstream csvFile;
		csvFile.open("sharpness.csv");
		for (const auto & entry : std::filesystem::directory_iterator(inputFolderName)) {
			int width, height, colorChannels;
			std::string path = entry.path().u8string();
			std::string fileName = path.substr(inputFolderName.length() + 1);
			unsigned char* data = stbi_load(path.c_str(), &width, &height, &colorChannels, 0);
			if (!data) {
				std::cerr << "ERROR: failed to load " << path << std::endl;
				std::cerr << stbi_failure_reason() << std::endl;
				continue;
			}
			float* bw = new float[width * height];
			int howManyAlphaPixels = 0;
			for (int y = 0; y < height; y++) {
				for (int x = 0; x < width; x++) {
					unsigned char* color = data + (y * width + x) * colorChannels;
					bw[y * width + x] = (color[0] + color[1] + color[2]) / 3.f / 255.f;
					if (colorChannels < 4 || color[3] > 127) howManyAlphaPixels++;
				}
			}
			float kernel[9] = {
				0.f, 1.f, 0.f,
				1.f, -4.f, 1.f,
				0.f, 1.f, 0.f
			};
			float* convolutedBw = new float[width * height];
			for (int y = 0; y < height; y++) {
				for (int x = 0; x < width; x++) {
					float sum = 0.f;
					for (int ky = -1; ky <= 1; ky++) {
						for (int kx = -1; kx <= 1; kx++) {
							int pixelX = clamp(x + kx, 0, width - 1);
							int pixelY = clamp(y + ky, 0, height - 1);
							sum += bw[pixelY * width + pixelX] * kernel[(ky + 1) * 3 + kx + 1];
						}
					}
					convolutedBw[y * width + x] = sum;
				}
			}

			float mean = 0.f;
			for (int y = 0; y < height; y++) {
				for (int x = 0; x < width; x++) {
					if (colorChannels < 4 || data[(y * width + x) * colorChannels + 3] > 127) mean += convolutedBw[y * width + x];
				}
			}
			mean /= (float)howManyAlphaPixels;
			float sum = 0.f;
			for (int y = 0; y < height; y++) {
				for (int x = 0; x < width; x++) {
					if (colorChannels < 4 || data[(y * width + x) * colorChannels + 3] > 127) sum += square(convolutedBw[y * width + x] - mean);
				}
			}
			float stdDev = sqrt(sum / (float)howManyAlphaPixels);

			csvFile << fileName << "," << stdDev << "\n";
			std::cout << fileName << " sharpness: " << stdDev << std::endl;
			//string savedFileName = "./" + outputFolderName + "/" + fileName;
			//cout << "Saved \"" << savedFileName << "\"" << endl;
			//stbi_write_png(savedFileName.c_str(), width, height, colorChannels, data, width * colorChannels);
			free(data);
            delete[] bw;
            delete[] convolutedBw;
		}
		csvFile.close();
    }*/
		std::cout << "webp and avif and some other formats dont work so you can convert them with imagemagick to png or jpeg" << std::endl;
		std::cout << "FOR SPRITESHEET SEPARATION===============" << std::endl;
		std::cout << "usage: atlaser separate [spritesheet file] [output image names] [start number] [makedivisiblebynumber] [edge margin] [alpha threshold] [min size]" << std::endl;
		std::cout << "if make divisible by number isnt 1 then it expands the image canvas so the width and height are divisible by that" << std::endl;
		std::cout << "alpha threshold is 0-255, any at or alpha below that is ignored" << std::endl;
		std::cout << "example: atlaser separate rockspritesheet.png Rock 1 4 2 25 10" << std::endl;
		std::cout << std::endl;
		std::cout << "FOR CROPPING AND STUFF SINGLE IMAGES===============" << std::endl;
		std::cout << "usage: atlaser image [input folder] [output folder] [alpha threshold] [makedivisiblebynumber] [edge margin] [crop to object true/false]" << std::endl;
		std::cout << "this processes every image in input folder and saves an image with the same name into output folder" << std::endl;
		std::cout << "alpha threshold is 0-255, any alpha at or below that is ignored" << std::endl;
		std::cout << "if make divisible by number isnt 1 then it expands the image canvas so the width and height are divisible by that" << std::endl;
		std::cout << "if crop to object is enabled then it removes the edges of the image that have no alpha" << std::endl;
		std::cout << "example: atlaser image 50 input output 4 2 true" << std::endl;
		std::cout << std::endl;
		std::cout << "FOR DOWNSCALING==================" << std::endl;
		std::cout << "usage: atlaser downscale [input folder] [output folder]" << std::endl;
		std::cout << std::endl;
    //}
	return 0;
}
