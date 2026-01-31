#pragma once
#include "Sprite.hpp"
#include <SDL.h>

class Animation {
public:
  const Sprite *sprite{nullptr};
  int currentFrame{0};
  float timer{0.0f};
  float frameDuration{0.1f};

  void update(float dt) {
    if (!sprite || sprite->totalFrames <= 1)
      return;

    timer += dt;
    if (timer >= frameDuration) {
      timer = 0.0f;
      currentFrame = (currentFrame + 1) % sprite->totalFrames;
    }
  }

  void draw(SDL_Renderer *renderer, int x, int y, int w, int h) const {
    if (!sprite)
      return;

    SDL_Rect src{
        .x = currentFrame * sprite->frameWidth,
        .y = 0,
        .w = sprite->frameWidth,
        .h = sprite->frameHeight,
    };

    SDL_Rect dst{.x = x, .y = y, .w = w, .h = h};
    SDL_RenderCopy(renderer, sprite->texture, &src, &dst);
  }
};
