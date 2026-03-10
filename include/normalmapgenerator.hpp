#ifndef NORMALMAPGENERATOR_H
#define NORMALMAPGENERATOR_H

#include "IImageData.hpp"
#include "ImageData.hpp"

class NormalMapGenerator
{
public:
    NormalMapGenerator();

    static ImageData<float, 1.f> generate(std::shared_ptr<IImageData> image);
};

#endif // NORMALMAPGENERATOR_H
