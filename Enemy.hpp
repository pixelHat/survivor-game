#pragma once
#include <SDL.h>

struct Enemy {
  float x, y;
  float speed{120.f};
  int health{1};
  bool active{true};

  void update(float playerX, float playerY, float dt);
  void draw(SDL_Renderer *renderer) const;

  SDL_Rect getRect() const {
    return {.x = (int)x, .y = (int)y, .w = 40, .h = 40};
  }
};
