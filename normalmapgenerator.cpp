#include "normalmapgenerator.hpp"
#include "math.hpp"
#include <chrono>

NormalMapGenerator::NormalMapGenerator() {}

ImageData<float, 1.f> NormalMapGenerator::generate(std::shared_ptr<IImageData> image) {
    float alphaThreshold = 0.25f;
    size_t w = image->getWidth();
    size_t h = image->getHeight();

    const std::vector<Vec2> checkOffsets = {
        Vec2(0, 1),
        Vec2(1, 0),
        Vec2(0, -1),
        Vec2(-1, 0)
    };

    auto start = std::chrono::high_resolution_clock::now();
    // get edge points
    std::vector<Vec2f> edgePoints;
    for (int y = 0; y < static_cast<int>(h); y += 1) {
        for (int x = 0; x < static_cast<int>(w); x += 1) {
            float alpha;
            image->getNormalizedPixelA(x, y, &alpha);
            if (alpha >= alphaThreshold) continue; // if pixel is solid then skip

            bool transparentNeighbor = false;
            for (Vec2 checkOffset : checkOffsets) {
                if (image->checkBounds(x + checkOffset.x, y + checkOffset.y)) {
                    float checkAlpha;
                    image->getNormalizedPixelA(x + checkOffset.x, y + checkOffset.y, &checkAlpha);
                    if (checkAlpha >= alphaThreshold) {
                        transparentNeighbor = true;
                        break;
                    }
                }
            }
            if (transparentNeighbor) {
                edgePoints.push_back(Vec2f(static_cast<float>(x), static_cast<float>(y)));
            }
        }
    }
    std::cout << "Edge points: " << edgePoints.size() << std::endl;
    auto end = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Edge point time: " << ms << std::endl;

    auto start2 = std::chrono::high_resolution_clock::now();
    // generate normal map
    const float edgeRadius = 40.f;
    ImageData<float, 1.f> normalMap{w, h, 3};
    for (int y = 0; y < static_cast<int>(h); y++) {
        float fy = static_cast<float>(y);
        for (int x = 0; x < static_cast<int>(w); x++) {
            float fx = static_cast<float>(x);

            float closestSquareDistance = 0.f;
            Vec2f closestPoint;
            bool first = true;
            for (Vec2f edgePoint : edgePoints) {
                float squareDistance = (fx - edgePoint.x) * (fx - edgePoint.x) + (fy - edgePoint.y) * (fy - edgePoint.y);
                if (first || squareDistance < closestSquareDistance) {
                    first = false;
                    closestSquareDistance = squareDistance;
                    closestPoint = edgePoint;
                }
            }

            float closestDistance = std::sqrt(closestSquareDistance);
            float t = std::min(closestDistance / edgeRadius, 1.f);
            if (1.f - t > 1e-5f) {
                Vec2f unitVector = (closestPoint - Vec2f(fx, fy)) / closestDistance;
                float slope = 1.f / (1.f - t) - 1.f;
                Vec3f normalVector{unitVector.x, unitVector.y, slope};
                normalVector.normalize();

                normalMap.setNormalizedPixelRGB(x, y, normalVector.x * 0.5f + 0.5f, normalVector.y * 0.5f + 0.5f, normalVector.z * 0.5f + 0.5f);
            } else { // flat region infinite slope
                normalMap.setNormalizedPixelRGB(x, y, 0.5f, 0.5f, 1.f);
            }
        }
    }
    auto end2 = std::chrono::high_resolution_clock::now();
    auto ms2 = std::chrono::duration_cast<std::chrono::milliseconds>(end2 - start2);
    std::cout << "Normal map time: " << ms2 << std::endl;


    return normalMap;
}
