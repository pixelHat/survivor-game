#include "Enemy.hpp"
#include <cmath>

void Enemy::update(float playerX, float playerY, float dt) {
  float diffX = playerX - x;
  float diffY = playerY - y;

  float distance = std::sqrt(diffX * diffX + diffY * diffY);

  if (distance > 0) {
    x += (diffX / distance) * speed * dt;
    y += (diffY / distance) * speed * dt;
  }
}

void Enemy::draw(SDL_Renderer *renderer) const {
  SDL_Rect rect = getRect();
  SDL_SetRenderDrawColor(renderer, 255, 50, 50, 255);
  SDL_RenderFillRect(renderer, &rect);
}
