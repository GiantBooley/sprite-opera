#ifndef SPRITEOPERA_H
#define SPRITEOPERA_H
#include "mainwindow.h"
#include <vector>
#include <sprite.hpp>

class SpriteOpera
{
public:
    MainWindow* window;
    std::vector<std::shared_ptr<VirtualSpritesheet>> spritesheets;
    int currentSpritesheetIndex;

    SpriteOpera();
    static SpriteOpera& inst();
};

#endif // SPRITEOPERA_H
