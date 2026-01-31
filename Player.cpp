#include "Player.hpp"
#include <algorithm>

void Player::handleInput(float dt) {
  const uint8_t *state = SDL_GetKeyboardState(nullptr);

  if (state[SDL_SCANCODE_W])
    y -= speed * dt;
  if (state[SDL_SCANCODE_S])
    y += speed * dt;
  if (state[SDL_SCANCODE_A])
    x -= speed * dt;
  if (state[SDL_SCANCODE_D])
    x += speed * dt;
}

void Player::update(float dt) {
  x = std::clamp(x, 0.0f, 800.0f - 50.0f);
  y = std::clamp(y, 0.0f, 600.0f - 50.0f);
  if (invincibilityTimer > 0) {
    invincibilityTimer -= dt;
  }
  if (xp >= xpToNextLevel) {
    level++;
    xp = 0;
    xpToNextLevel += 50;
    attackInterval *= 0.9f;
  }
  idleAnim.update(dt);
}

void Player::draw(SDL_Renderer *renderer) const {
  idleAnim.draw(renderer, (int)x, (int)y, 64, 64);
}

void Player::takeDamage(int damage) {
  if (invincibilityTimer <= 0) {
    life -= damage;
    invincibilityTimer = 0.5f;
  }
}
