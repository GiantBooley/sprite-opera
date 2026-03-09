#include "spriteopera.h"

SpriteOpera::SpriteOpera() : currentSpritesheetIndex(-1) {}

SpriteOpera& SpriteOpera::inst() {
    static SpriteOpera singleton;
    return singleton;
}
