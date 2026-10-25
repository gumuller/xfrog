// SPDX-License-Identifier: GPL-2.0-or-later
#include "render.hpp"
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#define STBI_NO_STDIO
#include "stb_image.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"
#include <array>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <stdexcept>
namespace xfrog {
static void checked(bool ok) {
  if (!ok)
    throw std::runtime_error(SDL_GetError());
}
static std::vector<unsigned char> bytes(const std::string &path) {
  std::ifstream file(std::filesystem::u8path(path), std::ios::binary);
  if (!file)
    throw std::runtime_error("Missing asset: " + path);
  return std::vector<unsigned char>((std::istreambuf_iterator<char>(file)), {});
}
struct Renderer::Font {
  SDL_Texture *texture = nullptr;
  stbtt_bakedchar chars[96]{};
  int width = 1024, height = 512;
  ~Font() { SDL_DestroyTexture(texture); }
};
Renderer::Font &Renderer::font(int size) {
  auto found = fonts.find(size);
  if (found != fonts.end())
    return *found->second;
  auto value = std::make_unique<Font>();
  if (size > 80) {
    value->width = 2048;
    value->height = 1024;
  }
  std::vector<unsigned char> bitmap(value->width * value->height);
  int result = stbtt_BakeFontBitmap(
      (size >= 34 ? headingData : fontData).data(), 0, float(size),
      bitmap.data(), value->width, value->height, 32, 96, value->chars);
  if (result <= 0)
    throw std::runtime_error("Font atlas overflow");
  std::vector<unsigned char> rgba(bitmap.size() * 4);
  for (size_t i = 0; i < bitmap.size(); ++i) {
    rgba[i * 4] = rgba[i * 4 + 1] = rgba[i * 4 + 2] = 255;
    rgba[i * 4 + 3] = bitmap[i];
  }
  value->texture =
      SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32,
                        SDL_TEXTUREACCESS_STATIC, value->width, value->height);
  checked(value->texture);
  checked(SDL_UpdateTexture(value->texture, nullptr, rgba.data(),
                            value->width * 4));
  SDL_SetTextureBlendMode(value->texture, SDL_BLENDMODE_BLEND);
  SDL_SetTextureScaleMode(value->texture, SDL_SCALEMODE_LINEAR);
  auto &reference = *value;
  fonts.emplace(size, std::move(value));
  return reference;
}
Image Renderer::image(const std::string &path, int columns, int rows) {
  auto source = bytes(path);
  Image value;
  int channels = 0;
  unsigned char *pixels =
      stbi_load_from_memory(source.data(), int(source.size()), &value.width,
                            &value.height, &channels, 4);
  if (!pixels)
    throw std::runtime_error("Invalid PNG: " + path);
  value.texture =
      SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32,
                        SDL_TEXTUREACCESS_STATIC, value.width, value.height);
  if (!value.texture) {
    stbi_image_free(pixels);
    throw std::runtime_error(SDL_GetError());
  }
  bool uploaded =
      SDL_UpdateTexture(value.texture, nullptr, pixels, value.width * 4);
  SDL_SetTextureBlendMode(value.texture, SDL_BLENDMODE_BLEND);
  SDL_SetTextureScaleMode(value.texture, SDL_SCALEMODE_LINEAR);
  if (columns > 1)
    for (int row = 0; row < rows; ++row)
      for (int col = 0; col < columns; ++col) {
        int x0 = col * value.width / columns,
            x1 = (col + 1) * value.width / columns,
            y0 = row * value.height / rows,
            y1 = (row + 1) * value.height / rows;
        int left = x1, right = x0, top = y1, bottom = y0;
        for (int y = y0; y < y1; ++y)
          for (int x = x0; x < x1; ++x)
            if (pixels[(y * value.width + x) * 4 + 3] > 32) {
              left = std::min(left, x);
              right = std::max(right, x);
              top = std::min(top, y);
              bottom = std::max(bottom, y);
            }
        if (left > right) {
          stbi_image_free(pixels);
          SDL_DestroyTexture(value.texture);
          throw std::runtime_error("Empty sprite cell in " + path);
        }
        value.frames.push_back({float(left), float(top),
                                float(right - left + 1),
                                float(bottom - top + 1)});
      }
  stbi_image_free(pixels);
  checked(uploaded);
  return value;
}
Renderer::Renderer(const std::string &assets, bool hidden) {
  checked(SDL_CreateWindowAndRenderer("Xfrog 2.0 - The Pizza Run", 1920, 1080,
                                      SDL_WINDOW_RESIZABLE |
                                          (hidden ? SDL_WINDOW_HIDDEN : 0),
                                      &window, &renderer));
  checked(SDL_SetRenderLogicalPresentation(renderer, int(Width), int(Height),
                                           SDL_LOGICAL_PRESENTATION_LETTERBOX));
  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
  SDL_SetRenderVSync(renderer, 1);
  framebuffer =
      SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32,
                        SDL_TEXTUREACCESS_TARGET, int(Width), int(Height));
  checked(framebuffer);
  SDL_SetTextureScaleMode(framebuffer, SDL_SCALEMODE_LINEAR);
  city = image(assets + "/city.png");
  courier = image(assets + "/courier.png", 8, 4);
  traffic = image(assets + "/traffic.png", 4, 2);
  poses = image(assets + "/poses.png", 4, 3);
  carryWalk = image(assets + "/carry-walk.png", 8, 4);
  carryPoses = image(assets + "/carry-poses.png", 4, 2);
  fontData = bytes(assets + "/Body.ttf");
  headingData = bytes(assets + "/LilitaOne.ttf");
  font(40);
  const int sizes[] = {15, 19, 21, 22, 23, 24, 25, 26, 28, 29, 30,  32, 33,
                       34, 40, 49, 58, 61, 64, 65, 70, 76, 92, 132, 142};
  int loaded = 0;
  for (int size : sizes) {
    font(size);
    ++loaded;
    if (loaded % 4 == 0 || loaded == 25) {
      SDL_SetRenderDrawColor(renderer, Ink.r, Ink.g, Ink.b, 255);
      SDL_RenderClear(renderer);
      text("Opening Frog City...", 960, 475, 40, Cream, true);
      round({660, 555, 600, 14}, 7, {46, 79, 83});
      round({660, 555, 600 * loaded / 25.f, 14}, 7, Mint);
      checked(SDL_RenderPresent(renderer));
      SDL_PumpEvents();
    }
  }
}
Renderer::~Renderer() {
  fonts.clear();
  SDL_DestroyTexture(city.texture);
  SDL_DestroyTexture(courier.texture);
  SDL_DestroyTexture(traffic.texture);
  SDL_DestroyTexture(poses.texture);
  SDL_DestroyTexture(carryWalk.texture);
  SDL_DestroyTexture(carryPoses.texture);
  SDL_DestroyTexture(framebuffer);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
}
void Renderer::setVsync(bool enabled) {
  SDL_SetRenderVSync(renderer, enabled ? 1 : 0);
}
void Renderer::fullscreen() {
  checked(SDL_SetWindowFullscreen(
      window, !(SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN)));
}
Vec Renderer::mousePosition(float x, float y) const {
  Vec p;
  SDL_RenderCoordinatesFromWindow(renderer, x, y, &p.x, &p.y);
  return p;
}
void Renderer::fill(Rect r, Color c) {
  SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
  SDL_FRect rect{r.x, r.y, r.w, r.h};
  SDL_RenderFillRect(renderer, &rect);
}
void Renderer::ellipse(Vec center, float rx, float ry, Color color) {
  std::array<SDL_Vertex, 42> vertices;
  std::array<int, 120> indices;
  vertices[0] = {
      {center.x, center.y},
      {color.r / 255.f, color.g / 255.f, color.b / 255.f, color.a / 255.f},
      {0, 0}};
  for (int i = 0; i <= 40; ++i) {
    float a = i * 6.283185f / 40;
    vertices[i + 1] = {
        {center.x + std::cos(a) * rx, center.y + std::sin(a) * ry},
        vertices[0].color,
        {0, 0}};
    if (i < 40) {
      indices[i * 3] = 0;
      indices[i * 3 + 1] = i + 1;
      indices[i * 3 + 2] = i + 2;
    }
  }
  SDL_RenderGeometry(renderer, nullptr, vertices.data(), int(vertices.size()),
                     indices.data(), int(indices.size()));
}
void Renderer::round(Rect r, float radius, Color color) {
  radius = std::min(radius, std::min(r.w, r.h) * .5f);
  if (radius <= 0) {
    fill(r, color);
    return;
  }
  std::array<SDL_Vertex, 38> vertices;
  std::array<int, 108> indices;
  SDL_FColor c{color.r / 255.f, color.g / 255.f, color.b / 255.f,
               color.a / 255.f};
  vertices[0] = {{r.x + r.w * .5f, r.y + r.h * .5f}, c, {0, 0}};
  int n = 1;
  for (int corner = 0; corner < 4; ++corner) {
    float cx = r.x + (corner == 0 || corner == 3 ? radius : r.w - radius),
          cy = r.y + (corner < 2 ? radius : r.h - radius);
    float start = 3.14159265f + corner * 1.5707963f;
    for (int step = 0; step <= 8; ++step) {
      float a = start + step * 1.5707963f / 8;
      vertices[n++] = {
          {cx + std::cos(a) * radius, cy + std::sin(a) * radius}, c, {0, 0}};
    }
  }
  vertices[n] = vertices[1];
  for (int i = 0; i < 36; ++i) {
    indices[i * 3] = 0;
    indices[i * 3 + 1] = i + 1;
    indices[i * 3 + 2] = i + 2;
  }
  SDL_RenderGeometry(renderer, nullptr, vertices.data(), 38, indices.data(),
                     108);
}
float Renderer::textWidth(const std::string &string, int size) {
  auto &f = font(size);
  float result = 0;
  for (unsigned char ch : string)
    if (ch >= 32 && ch < 128)
      result += f.chars[ch - 32].xadvance;
  return result;
}
void Renderer::text(const std::string &string, float x, float y, int size,
                    Color color, bool centered) {
  auto &f = font(size);
  if (centered)
    x -= textWidth(string, size) * .5f;
  float cursor = x, baseline = y + size * .78f;
  std::vector<SDL_Vertex> vertices;
  std::vector<int> indices;
  vertices.reserve(string.size() * 4);
  indices.reserve(string.size() * 6);
  SDL_FColor tint{color.r / 255.f, color.g / 255.f, color.b / 255.f,
                  color.a / 255.f};
  for (unsigned char ch : string) {
    if (ch < 32 || ch >= 128)
      continue;
    stbtt_aligned_quad q;
    stbtt_GetBakedQuad(f.chars, f.width, f.height, ch - 32, &cursor, &baseline,
                       &q, 1);
    int offset = int(vertices.size());
    vertices.push_back({{q.x0, q.y0}, tint, {q.s0, q.t0}});
    vertices.push_back({{q.x1, q.y0}, tint, {q.s1, q.t0}});
    vertices.push_back({{q.x1, q.y1}, tint, {q.s1, q.t1}});
    vertices.push_back({{q.x0, q.y1}, tint, {q.s0, q.t1}});
    indices.insert(indices.end(), {offset, offset + 1, offset + 2, offset,
                                   offset + 2, offset + 3});
  }
  if (!vertices.empty())
    checked(SDL_RenderGeometry(renderer, f.texture, vertices.data(),
                               int(vertices.size()), indices.data(),
                               int(indices.size())));
}
void Renderer::picture(const Image &img, Rect target, const SDL_FRect *source,
                       float alpha) {
  SDL_SetTextureAlphaMod(img.texture, Uint8(std::clamp(alpha, 0.f, 1.f) * 255));
  SDL_FRect r{target.x, target.y, target.w, target.h};
  SDL_RenderTexture(renderer, img.texture, source, &r);
}
void Renderer::panel(Rect r, Color color) {
  round({r.x, r.y + 8, r.w, r.h}, 24, {7, 24, 33, 75});
  round(r, 24, color);
}
void Renderer::button(const std::string &label, Rect r,
                      const std::string &action, Vec mouse, bool primary) {
  bool hot = r.contains(mouse);
  Color color = primary ? (hot ? Color{255, 141, 117} : Coral)
                        : (hot ? Color{64, 104, 105} : Color{42, 77, 81});
  round({r.x, r.y + 6, r.w, r.h}, 18, {7, 25, 32, 90});
  round(r, 18, color);
  text(label, r.x + r.w * .5f, r.y + r.h * .5f - 23, 40, primary ? Ink : Cream,
       true);
  buttons.push_back({r, action});
}
std::string Renderer::clicked(Vec p) const {
  for (auto it = buttons.rbegin(); it != buttons.rend(); ++it)
    if (it->rect.contains(p))
      return it->action;
  return {};
}
void Renderer::courierSprite(Vec feet, int facing, float frame, float height,
                             float alpha, int pose, bool holding) {
  int index = std::clamp(facing, 0, 3) * 8 + (int(frame) % 8);
  const Image &atlas = holding ? (pose >= 0 ? carryPoses : carryWalk)
                               : (pose >= 0 ? poses : courier);
  const auto &src = atlas.frames[pose >= 0 ? pose : index];
  float w = height * src.w / src.h;
  ellipse({feet.x, feet.y + 3}, height * .22f, height * .065f,
          {17, 43, 50, Uint8(75 * alpha)});
  picture(atlas, {feet.x - w * .5f, feet.y - height, w, height}, &src, alpha);
}
void Renderer::vehicle(const Car &car, float time) {
  const auto &src = traffic.frames[car.style + (car.speed < 0 ? 4 : 0)];
  float w = car.width + 30, h = w * src.h / src.w;
  ellipse({car.x, car.y + 36}, w * .45f, 12, {9, 27, 35, 70});
  picture(traffic,
          {car.x - w * .5f,
           car.y + 44 - h + std::sin(time * 8 + car.style * 1.7f) * .7f, w, h},
          &src);
}
void Renderer::board(const Game &game, float time) {
  Vec feet = game.run.previous * (1 - blend) + game.run.player * blend;
  picture(city, {0, 0, Width, Height});
  if (game.screen != Screen::Menu) {
    Vec destination =
        game.run.carrying ? game.doors[game.run.target] : game.shop;
    float pulse = std::sin(time * 4) * .5f + .5f;
    ellipse(destination, 61 + pulse * 5, 19 + pulse * 2, {255, 214, 105, 60});
    ellipse(destination, 40, 12, {255, 225, 140, 95});
    // Clear, diegetic destination tag anchored at the door.
    float width = game.run.carrying ? 270.f : 220.f;
    if (!game.canInteract()) {
      round({destination.x - width * .5f, destination.y - 79, width, 42}, 16,
            Ink);
      text(game.run.carrying ? "DELIVER HERE" : "PIZZA PICKUP", destination.x,
           destination.y - 75, 28, Gold, true);
      text("v", destination.x, destination.y - 41, 32, Gold, true);
    }
    if (game.run.carrying) {
      float alpha = .65f + .35f * std::sin(time * 3);
      ellipse({feet.x, feet.y + 2}, 30, 10, {255, 207, 93, Uint8(100 * alpha)});
    }
    // Sort foot positions so the courier can pass both behind and in front of
    // cars.
    struct DrawItem {
      float y;
      int index;
    };
    std::vector<DrawItem> order;
    for (size_t i = 0; i < game.run.cars.size(); ++i)
      order.push_back({game.run.cars[i].y + 40, int(i)});
    order.push_back({feet.y, -1});
    std::sort(order.begin(), order.end(),
              [](DrawItem a, DrawItem b) { return a.y < b.y; });
    for (auto item : order)
      if (item.index < 0) {
        float alpha = game.run.invulnerable > 0
                          ? (.55f + .45f * std::abs(std::sin(time * 15)))
                          : 1;
        if (game.run.dashTime > 0) {
          for (int n = 3; n > 0; --n)
            courierSprite(feet - game.run.velocity * (n * .018f),
                          game.run.facing, game.run.animation, 110, .13f, -1,
                          game.run.carrying);
        }
        int pose = -1;
        bool holding = game.run.carrying;
        if (game.run.invulnerable > 1.6f) {
          holding = false;
          pose = 8 + std::min(3, int((2 - game.run.invulnerable) * 10));
        } else if (length(game.run.velocity) < 20) {
          if (game.run.handoff > 0) {
            holding = true;
            pose = 4 + std::clamp(int((.36f - game.run.handoff) / .09f), 0, 3);
          } else if (game.run.pickup > 0) {
            pose = 7 - std::clamp(int((.32f - game.run.pickup) / .08f), 0, 3);
          } else if (holding) {
            pose = game.run.facing;
          } else {
            pose = game.run.celebration > 0
                       ? 4 + std::clamp(int((.44f - game.run.celebration) * 9),
                                        0, 3)
                       : game.run.facing;
          }
        }
        courierSprite(feet, game.run.facing, game.run.animation, 110, alpha,
                      pose, holding);
      } else {
        Car car = game.run.cars[item.index];
        car.x -= car.speed * ((1 - blend) / 120.f);
        vehicle(car, time);
      }
    for (const auto &p : game.particles)
      ellipse(
          p.p, p.size, p.size,
          rgb(p.color, Uint8(255 * std::clamp(p.life / p.total, 0.f, 1.f))));
    for (const auto &f : game.floaters) {
      round({f.p.x - 260, f.p.y - 3, 520, 43}, 16,
            {20, 48, 54, Uint8(220 * std::min(f.life, 1.f))});
      text(f.text, f.p.x, f.p.y + 2, 26,
           rgb(f.color, Uint8(255 * std::min(f.life, 1.f))), true);
    }
    if (game.run.shake > 0)
      fill({0, 0, Width, Height}, {255, 137, 112, Uint8(game.run.shake * 80)});
  }
}
void Renderer::hud(const Game &game, float time) {
  const auto &r = game.run;
  panel({28, 22, 1864, 100}, {20, 48, 55, 245});
  text("XFROG", 55, 31, 58, Cream);
  round({247, 50, 63, 29}, 10, Coral);
  text("2.0", 278, 50, 22, Ink, true);
  text("SHIFT " + std::to_string(r.shift), 350, 37, 23, Mint);
  text(std::to_string(r.delivered) + " / " + std::to_string(r.goal) +
           " DELIVERIES",
       350, 65, 30, Cream);
  fill({680, 44, 2, 58}, {255, 245, 220, 35});
  text("SCORE", 715, 37, 22, Mint);
  text(std::to_string(r.score), 715, 62, 34, Cream);
  text(r.carrying ? game.shops[r.target] : "PICK UP A FRESH PIZZA", 940, 34, 25,
       Cream);
  round({940, 79, 350, 10}, 5, {61, 85, 84, 255});
  round({940, 79, std::max(10.f, 350 * (r.carrying ? r.heat : 1.f)), 10}, 5,
        r.carrying && r.heat < .3f ? Coral : Gold);
  text(r.carrying ? "PIZZA HEAT" : "ONE ORDER AT A TIME", 940, 95, 15, Mint);
  int seconds = int(std::ceil(r.time));
  std::string clock = std::to_string(seconds / 60) + ":" +
                      (seconds % 60 < 10 ? "0" : "") +
                      std::to_string(seconds % 60);
  text(clock, 1450, 37, 49, seconds < 30 ? Coral : Cream, true);
  for (int i = 0; i < r.hearts; ++i) {
    float x = 1550.f + i * 43;
    ellipse({x - 7, 64}, 11, 11, Coral);
    ellipse({x + 7, 64}, 11, 11, Coral);
    SDL_Vertex v[3] = {{{x - 17, 67},
                        {Coral.r / 255.f, Coral.g / 255.f, Coral.b / 255.f, 1},
                        {0, 0}},
                       {{x + 17, 67},
                        {Coral.r / 255.f, Coral.g / 255.f, Coral.b / 255.f, 1},
                        {0, 0}},
                       {{x, 88},
                        {Coral.r / 255.f, Coral.g / 255.f, Coral.b / 255.f, 1},
                        {0, 0}}};
    SDL_RenderGeometry(renderer, nullptr, v, 3, nullptr, 0);
  }
  text("ESC  PAUSE", 1803, 54, 21, Mint, true);
  panel({34, 944, 294, 94}, {20, 48, 55, 240});
  text("SHIFT  /  DASH", 57, 956, 26, Cream);
  float charge = 1 - r.dashCooldown / 1.65f;
  round({57, 999, 244, 10}, 5, {62, 88, 85});
  round({57, 999, std::max(8.f, 244 * charge), 10}, 5,
        charge > .99f ? Mint : Gold);
  if (r.combo > 1) {
    panel({1535, 945, 350, 91}, {20, 48, 55, 240});
    text(std::to_string(r.combo) + " DELIVERY STREAK", 1710, 961, 29, Gold,
         true);
    text("KEEP IT HOT. KEEP IT GOING.", 1710, 998, 19, Mint, true);
  } else
    text("ARROWS / WASD  MOVE     SPACE  PICK UP / DELIVER     H  HELP", 1265,
         1040, 23, Ink, true);
  if (game.canInteract()) {
    round({r.player.x - 178, r.player.y - 157, 356, 46}, 18, Gold);
    text(r.carrying ? "SPACE  DELIVER PIZZA" : "SPACE  GRAB A PIZZA",
         r.player.x, r.player.y - 151, 29, Ink, true);
  }
  // Incoming traffic hints occupy only road edges, away from the playable gaps.
  for (int lane = 0; lane < 3; ++lane) {
    float y = 416.f + lane * 100;
    bool left = lane != 1;
    float edge = left ? 20.f : 1900.f;
    float a = .4f + .4f * std::sin(time * 3 + lane);
    ellipse({edge, y + 18}, 18, 18, {255, 208, 109, Uint8(160 * a)});
    text(left ? ">" : "<", edge, y - 1, 33, Gold, true);
  }
}
void Renderer::menu(const Game &game, float time, Vec mouse) {
  fill({0, 0, Width, Height}, {9, 34, 45, 115});
  panel({105, 105, 800, 865}, {19, 48, 57, 247});
  round({153, 143, 348, 40}, 16, {50, 87, 87});
  text("FROG CITY  /  THE PIZZA RUN", 327, 148, 25, Mint, true);
  text("Xfrog", 151, 194, 142, Cream);
  text("2.0", 572, 232, 70, Coral);
  text("Hot pizza. Cool head.", 155, 363, 49, Gold);
  text("A sunny town. Three busy lanes.", 155, 432, 32, Cream);
  text("One very determined delivery kid.", 155, 470, 32, Cream);
  button("START SHIFT", {153, 548, 704, 95}, "start", mouse, true);
  text("CHOOSE YOUR PACE", 155, 674, 23, Mint);
  bool relaxed = game.profile.difficulty == Difficulty::Relaxed;
  button(relaxed ? "RELAXED  *" : "RELAXED", {153, 716, 340, 70}, "relaxed",
         mouse);
  button(!relaxed ? "ARCADE  *" : "ARCADE", {517, 716, 340, 70}, "arcade",
         mouse);
  text(relaxed ? "Room to breathe. Five hearts. Longer shifts."
               : "Faster decisions. Three hearts. Bigger challenge.",
       155, 810, 25, Cream);
  button("HOW TO PLAY", {153, 863, 340, 62}, "help", mouse);
  button(game.profile.sound ? "SOUND ON" : "SOUND OFF", {517, 863, 340, 62},
         "sound", mouse);
  panel({1120, 146, 613, 225}, {255, 245, 220, 242});
  text("SPECIAL DELIVERY", 1426, 168, 24, Ink, true);
  text("Good food.", 1426, 215, 64, Ink, true);
  text("Great timing.", 1426, 281, 64, Coral, true);
  courierSprite({1420, 850}, 0, 0, 370 + std::sin(time * 2) * 2, 1, 0);
  round({1170, 906, 520, 67}, 24, {19, 48, 57, 235});
  text("PERSONAL BEST  " + std::to_string(game.profile.best), 1430, 920, 33,
       Gold, true);
  text(
      "ENTER  START     H  HELP     M  SOUND     F11  FULLSCREEN     ESC  QUIT",
      960, 1034, 23, Cream, true);
}
void Renderer::overlay(const Game &game, Vec mouse) {
  fill({0, 0, Width, Height}, {9, 28, 38, 180});
  if (game.screen == Screen::Help) {
    panel({400, 140, 1120, 805}, {22, 49, 57, 252});
    text("YOUR NEXT GREAT DELIVERY", 960, 180, 61, Cream, true);
    const char *titles[] = {"01  PICK IT UP", "02  FIND THE GAP",
                            "03  DELIVER IT HOT"};
    const char *lines[] = {
        "Start at the pizza shop. Press SPACE to take an order.",
        "Walk around the shop, watch traffic, and cross to the glowing door.",
        "Press SPACE at the marked door. Hotter pizzas earn bigger tips."};
    for (int i = 0; i < 3; ++i) {
      text(titles[i], 470, 290 + i * 130, 34, Gold);
      text(lines[i], 470, 341 + i * 130, 28, Cream);
    }
    text("MOVE  ARROWS / WASD       DASH  SHIFT       PAUSE  ESC / P", 960, 728,
         28, Mint, true);
    text("Dash has a short cooldown. Watch your pizza heat and your hearts.",
         960, 780, 26, Cream, true);
    button("GOT IT. LET'S GO!", {660, 842, 600, 70}, "closehelp", mouse, true);
  } else if (game.screen == Screen::Paused) {
    panel({600, 235, 720, 610}, {22, 49, 57, 250});
    text("TAKE A BREATHER", 960, 275, 65, Cream, true);
    text("Your shift is paused. Pizza stays warm.", 960, 367, 30, Mint, true);
    button("BACK TO THE RUN", {665, 444, 590, 85}, "resume", mouse, true);
    button(game.profile.sound ? "SOUND ON" : "SOUND OFF", {665, 557, 590, 70},
           "sound", mouse);
    button("END SHIFT / MENU", {665, 655, 590, 70}, "menu", mouse);
    text("ESC / ENTER  RESUME     H  HELP", 960, 778, 24, Cream, true);
  } else if (game.screen == Screen::Results) {
    bool won = game.run.won;
    panel({505, 155, 910, 780}, {22, 49, 57, 250});
    text(won ? "SHIFT COMPLETE!" : "THAT'S A WRAP!", 960, 198, 76,
         won ? Gold : Cream, true);
    text(won ? "Fresh pizzas. Happy neighbors. Nice work!"
             : "A little traffic never stopped a great courier.",
         960, 298, 30, Mint, true);
    text(std::to_string(game.run.score), 960, 366, 132, Cream, true);
    text("POINTS THIS SHIFT", 960, 500, 26, Gold, true);
    text(std::to_string(game.run.delivered) + " DELIVERIES     " +
             std::to_string(game.run.bestCombo) + " BEST STREAK     " +
             std::to_string(game.run.nearMisses) + " CLOSE CALLS",
         960, 569, 29, Cream, true);
    text("PERSONAL BEST  " + std::to_string(game.profile.best), 960, 632, 32,
         Mint, true);
    button(won ? "NEXT SHIFT" : "TRY AGAIN", {625, 712, 670, 84}, "next", mouse,
           true);
    button("BACK TO MENU", {625, 825, 670, 66}, "menu", mouse);
  }
}
void Renderer::draw(const Game &game, float elapsed, Vec mouse,
                    float interpolation) {
  blend =
      game.screen == Screen::Playing ? std::clamp(interpolation, 0.f, 1.f) : 1;
  buttons.clear();
  checked(SDL_SetRenderTarget(renderer, framebuffer));
  SDL_SetRenderDrawColor(renderer, 13, 34, 45, 255);
  SDL_RenderClear(renderer);
  board(game, elapsed);
  if (game.screen == Screen::Menu)
    menu(game, elapsed, mouse);
  else {
    hud(game, elapsed);
    if (game.screen != Screen::Playing)
      overlay(game, mouse);
  }
  checked(SDL_SetRenderTarget(renderer, nullptr));
  SDL_SetRenderDrawColor(renderer, 13, 34, 45, 255);
  SDL_RenderClear(renderer);
  SDL_RenderTexture(renderer, framebuffer, nullptr, nullptr);
  checked(SDL_RenderPresent(renderer));
}
void Renderer::capture(const std::string &path) {
  checked(SDL_SetRenderTarget(renderer, framebuffer));
  SDL_Surface *surface = SDL_RenderReadPixels(renderer, nullptr);
  SDL_SetRenderTarget(renderer, nullptr);
  if (!surface)
    throw std::runtime_error(SDL_GetError());
  bool ok = SDL_SaveBMP(surface, path.c_str());
  SDL_DestroySurface(surface);
  checked(ok);
}
} // namespace xfrog
