#pragma once
#include <SDL.h>

class Player {
public:
  float x{100.0f}, y{300.0f};
  float speed{300.0f};
  float attackTimer{0.0f};
  int life{100};
  float maxLife{100.0f};
  float invincibilityTimer{0.0f};
  int xp{0};
  int level{1};
  int xpToNextLevel{100};
  float attackInterval{1.0f};

  void handleInput(float dt);
  void update(float dt);
  void draw(SDL_Renderer *renderer) const;
  void takeDamage(int damage);

  SDL_Rect getRect() const {
    return {.x = (int)x, .y = (int)y, .w = 50, .h = 50};
  }
};
