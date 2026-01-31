#include "Enemy.hpp"
#include "Player.hpp"
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <algorithm>
#include <format>
#include <iostream>
#include <limits>
#include <memory>
#include <random>
#include <string>
#include <vector>

bool checkCollision(const SDL_Rect &a, const SDL_Rect &b) {
  return (a.x < b.x + b.w && a.x + a.w > b.x && a.y < b.y + b.h &&
          a.y + a.h > b.y);
}

Enemy *findClosestEnemy(Player &p, std::vector<Enemy> &enemies) {
  Enemy *closest = nullptr;
  float minDistance = std::numeric_limits<float>::max();

  for (auto &e : enemies) {
    if (!e.active)
      continue;

    float dx = e.x - p.x;
    float dy = e.y - p.y;
    float distSq = dx * dx + dy * dy;
    // we can skeep the sqrt because we just want the biggest distance.
    // We don't care about the value itself.

    if (distSq < minDistance) {
      minDistance = distSq;
      closest = &e;
    }
  }
  return closest;
}

void drawText(SDL_Renderer *renderer, TTF_Font *font, std::string text, int x,
              int y) {
  SDL_Color white = {255, 255, 255, 255};
  auto *surface = TTF_RenderText_Blended(font, text.c_str(), white);
  auto *texture = SDL_CreateTextureFromSurface(renderer, surface);
  SDL_Rect dest = {.x = x, .y = y, .w = surface->w, .h = surface->h};
  SDL_RenderCopy(renderer, texture, nullptr, &dest);
  SDL_FreeSurface(surface);
  SDL_DestroyTexture(texture);
}

enum class GameState { Playing, GameOver };

struct Rect {
  int x, y, width, height;

  [[nodiscard]] constexpr bool contains(int px, int py) const noexcept {
    return (px > x && px < width) && (py > y && py < height);
  }
};

struct Projectile {
  float x, y;
  float velX, velY;
  bool active{true};
};

struct Gem {
  float x, y;
  int xpValue{10};
  bool active{true};

  SDL_Rect getRect() const {
    return {.x = (int)x, .y = (int)y, .w = 15, .h = 15};
  }
};

std::vector<Projectile> bullets;
std::vector<Enemy> enemies;
std::vector<Gem> gems;
float enemySpawnTimer{0.0f};

auto main(int argc, char *argv[]) -> int {
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    std::cout << "SDL could not initialize! SDL_Error: " << SDL_GetError()
              << std::endl;
    return 1;
  }

  if (TTF_Init() == -1) {
    return 2;
  }

  TTF_Font *font = TTF_OpenFont("assets/font.ttf", 24);
  if (!font) {
    std::cout << "Failed to load font!" << std::endl;
  }

  IMG_Init(IMG_INIT_PNG);

  int mouseX, mouseY;
  SDL_GetGlobalMouseState(&mouseX, &mouseY);
  auto pointDisplayIndex = SDL_Point{mouseX, mouseY};
  int displayIndex = SDL_GetPointDisplayIndex(&pointDisplayIndex);
  if (displayIndex < 0)
    displayIndex = 0;

  constexpr Rect screen_bounds{.x = 0, .y = 0, .width = 800, .height = 600};
  auto *window = SDL_CreateWindow(
      "My C++ Game", SDL_WINDOWPOS_CENTERED_DISPLAY(displayIndex),
      SDL_WINDOWPOS_CENTERED_DISPLAY(displayIndex), screen_bounds.width,
      screen_bounds.height, SDL_WINDOW_SHOWN);

  auto *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> disX(0, 800);
  std::uniform_int_distribution<> disY(0, 600);

  std::vector<Projectile> bullets;
  bool isRunning{true};
  SDL_Event event;

  uint32_t lastFrameTime = SDL_GetTicks();
  auto currentState = GameState::Playing;

  // Load the player texture
  SDL_Texture *texLeft =
      IMG_LoadTexture(renderer, "assets/player/player_idle_left.png");
  SDL_Texture *texRight =
      IMG_LoadTexture(renderer, "assets/player/player_idle_right.png");
  SDL_Texture *texUp =
      IMG_LoadTexture(renderer, "assets/player/player_idle_back.png");
  SDL_Texture *texDown =
      IMG_LoadTexture(renderer, "assets/player/player_idle_front.png");

  Player player{
      .x = 400.0f,
      .y = 300.0f,
  };

  player.spriteLibrary[Direction::Left] = {.texture = texLeft,
                                           .frameWidth = 32,
                                           .frameHeight = 32,
                                           .totalFrames = 9};
  player.spriteLibrary[Direction::Right] = {.texture = texRight,
                                            .frameWidth = 32,
                                            .frameHeight = 32,
                                            .totalFrames = 9};

  player.spriteLibrary[Direction::Up] = {
      .texture = texUp, .frameWidth = 32, .frameHeight = 32, .totalFrames = 9};
  player.spriteLibrary[Direction::Down] = {.texture = texDown,
                                           .frameWidth = 32,
                                           .frameHeight = 32,
                                           .totalFrames = 9};
  player.idleAnim.frameDuration = 0.12f;

  while (isRunning) {
    uint32_t currentFrameTime = SDL_GetTicks();
    float dt = (currentFrameTime - lastFrameTime) / 1000.0f;
    lastFrameTime = currentFrameTime;

    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        isRunning = false;
      }
      if (event.type == SDL_KEYDOWN) {
        if (currentState == GameState::GameOver &&
            event.key.keysym.scancode == SDL_SCANCODE_R) {
          player.life = player.maxLife;
          player.xp = 0;
          player.level = 1;
          player.x = 400;
          player.y = 300;

          enemies.clear();
          bullets.clear();
          gems.clear();

          currentState = GameState::Playing;
        }
      }
    }

    if (currentState == GameState::Playing) {
      player.handleInput(dt);
      player.update(dt);
      player.attackTimer += dt;
      if (player.attackTimer >= player.attackInterval) {
        if (auto *target = findClosestEnemy(player, enemies)) {
          if (screen_bounds.contains(target->x, target->y)) {
            float dx = target->x - player.x;
            float dy = target->y - player.y;
            float distance = std::sqrt(dx * dx + dy * dy);
            float bulletSpeed = 600.0f;
            bullets.push_back(
                Projectile{.x = player.x + 25,
                           .y = player.y + 25,
                           .velX = (dx / distance) * bulletSpeed,
                           .velY = (dy / distance) * bulletSpeed});
          }
        }
        player.attackTimer = 0.0f;
      }

      for (auto &b : bullets) {
        b.x += b.velX * dt;
        b.y += b.velY * dt;
      }

      enemySpawnTimer += dt;
      if (enemySpawnTimer >= 1.5f) {
        float x = disX(gen);
        float y = disY(gen);
        float dx = x - player.x;
        float dy = y - player.y;
        float distSq = std::sqrt(dx * dx + dy * dy);
        if (distSq <= 100) {
          x = 0;
          y = 0;
        }
        enemies.push_back(Enemy{.x = x, .y = y});
        enemySpawnTimer = 0.0f;
      }
      for (auto &e : enemies) {
        e.update(player.x, player.y, dt);
      }

      //== Collision
      for (auto &b : bullets) {
        if (!b.active)
          continue;

        SDL_Rect bulletRect{.x = (int)b.x, .y = (int)b.y, .w = 10, .h = 5};

        for (auto &e : enemies) {
          if (!e.active)
            continue;

          SDL_Rect enemyRect = e.getRect();

          if (SDL_HasIntersection(&bulletRect, &enemyRect)) {
            b.active = false;
            e.active = false;
            gems.push_back(Gem{.x = e.x + 10, .y = e.y + 10});
            break;
          }
        }
      }

      SDL_Rect playerRect = player.getRect();
      for (auto &e : enemies) {
        if (!e.active)
          continue;
        SDL_Rect enemyRect = e.getRect();
        if (SDL_HasIntersection(&playerRect, &enemyRect)) {
          e.active = false;
          player.takeDamage(40);
        }
      }
      for (auto &gem : gems) {
        SDL_Rect gemRect = gem.getRect();
        if (SDL_HasIntersection(&playerRect, &gemRect)) {
          gem.active = false;
          player.xp += gem.xpValue;
        }
      }

      if (player.life <= 0) {
        currentState = GameState::GameOver;
      }
    }

    //== Clean memory
    std::erase_if(bullets, [](const auto &b) {
      return b.x > 800 || b.x < 0 || b.y > 600 || b.y < 0 || !b.active;
    });
    std::erase_if(enemies, [](const auto &e) { return !e.active; });
    std::erase_if(gems, [](const auto &g) { return !g.active; });

    //== Render
    if (currentState == GameState::Playing) {
      SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
      SDL_RenderClear(renderer);

      player.draw(renderer);
      for (const auto &e : enemies) {
        e.draw(renderer);
      }

      // Draw Bullets
      SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
      for (const auto &b : bullets) {
        SDL_Rect r{.x = (int)b.x, .y = (int)b.y, .w = 10, .h = 5};
        SDL_RenderFillRect(renderer, &r);
      }

      // Draw Gems
      SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255);
      for (const auto &g : gems) {
        SDL_Rect r = g.getRect();
        SDL_RenderFillRect(renderer, &r);
      }

      int barWidth = 50;
      int barHeight = 8;
      int barX = static_cast<int>(player.x);
      int barY = static_cast<int>(player.y) - 15;
      float tmp = player.maxLife;
      float healthPercent =
          std::clamp(static_cast<float>(player.life) / tmp, 0.0f, 100.0f);
      int greenBarWidth = static_cast<int>(barWidth * healthPercent);

      // Health Background
      SDL_Rect bgRect{.x = barX, .y = barY, .w = barWidth, .h = barHeight};
      SDL_SetRenderDrawColor(renderer, 200, 0, 0, 255);
      SDL_RenderFillRect(renderer, &bgRect);
      // Health Foreground
      SDL_Rect fgRect{.x = barX, .y = barY, .w = greenBarWidth, .h = barHeight};
      SDL_SetRenderDrawColor(renderer, 0, 200, 0, 255);
      SDL_RenderFillRect(renderer, &fgRect);

      // Draw Score
      std::string hudText = std::format("Level: {} | XP: {}/{}", player.level,
                                        player.xp, player.xpToNextLevel);
      drawText(renderer, font, hudText, 20, 20);
    } else if (currentState == GameState::GameOver) {
      SDL_SetRenderDrawColor(renderer, 0, 0, 0, 150);
      SDL_RenderFillRect(renderer, nullptr);
      drawText(renderer, font, "GAME OVER", 330, 250);
      drawText(renderer, font, "Press R to Restart", 280, 300);
    }
    SDL_RenderPresent(renderer);
  }

  // Clean Up
  TTF_CloseFont(font);
  TTF_Quit();
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}
