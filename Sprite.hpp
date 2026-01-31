#pragma once
#include <SDL.h>

struct Sprite {
  SDL_Texture *texture{nullptr};
  int frameWidth{0};
  int frameHeight{0};
  int totalFrames{0};
};
