// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include "game.hpp"
#include <SDL3/SDL.h>
#include <map>
#include <memory>
#include <string>
#include <vector>
namespace xfrog {
struct Color {
  Uint8 r, g, b, a = 255;
};
inline Color rgb(uint32_t color, Uint8 alpha = 255) {
  return {Uint8(color >> 16), Uint8(color >> 8), Uint8(color), alpha};
}
constexpr Color Ink{22, 49, 57}, Cream{255, 245, 220}, Mint{139, 232, 195},
    Coral{247, 113, 99}, Gold{255, 208, 109};
struct Image {
  SDL_Texture *texture = nullptr;
  int width = 0, height = 0;
  std::vector<SDL_FRect> frames;
};
struct Button {
  Rect rect;
  std::string action;
};
class Renderer {
public:
  SDL_Window *window = nullptr;
  SDL_Renderer *renderer = nullptr;
  std::vector<Button> buttons;
  Renderer(const std::string &assets, bool hidden = false);
  ~Renderer();
  void draw(const Game &game, float elapsed, Vec mouse);
  void capture(const std::string &path);
  void fullscreen();
  void setVsync(bool enabled);
  std::string clicked(Vec point) const;
  Vec mousePosition(float x, float y) const;
  void text(const std::string &string, float x, float y, int size, Color color,
            bool centered = false);
  float textWidth(const std::string &string, int size);
  void round(Rect r, float radius, Color color);
  void ellipse(Vec center, float rx, float ry, Color color);
  void fill(Rect r, Color color);

private:
  struct Font;
  std::vector<unsigned char> fontData, headingData;
  std::map<int, std::unique_ptr<Font>> fonts;
  SDL_Texture *framebuffer = nullptr;
  Image city, courier, traffic;
  std::vector<Image *> images;
  Font &font(int size);
  Image image(const std::string &path, int columns = 1, int rows = 1);
  void picture(const Image &img, Rect target, const SDL_FRect *source = nullptr,
               float alpha = 1);
  void courierSprite(Vec feet, int facing, float frame, float height = 110,
                     float alpha = 1);
  void vehicle(const Car &car, float time);
  void panel(Rect r, Color color);
  void button(const std::string &label, Rect r, const std::string &action,
              Vec mouse, bool primary = false);
  void board(const Game &game, float time);
  void menu(const Game &game, float time, Vec mouse);
  void hud(const Game &game, float time);
  void overlay(const Game &game, Vec mouse);
};
} // namespace xfrog
