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
Image Renderer::image(const std::string &path, int columns, int rows,
                      bool components) {
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
  SDL_SetTextureBlendMode(value.texture, SDL_BLENDMODE_BLEND);
  SDL_SetTextureScaleMode(value.texture, SDL_SCALEMODE_LINEAR);
  if (components) {
    struct Part {
      int label, count, left, top, right, bottom;
    };
    std::vector<int> labels(value.width * value.height, -1), queue;
    std::vector<Part> parts;
    int label = 0;
    for (int y = 0; y < value.height; ++y)
      for (int x = 0; x < value.width; ++x) {
        int start = y * value.width + x;
        if (labels[start] >= 0 || pixels[start * 4 + 3] <= 32)
          continue;
        Part part{label, 0, x, y, x, y};
        queue.clear();
        queue.push_back(start);
        labels[start] = label;
        for (size_t at = 0; at < queue.size(); ++at) {
          int point = queue[at], px = point % value.width,
              py = point / value.width;
          ++part.count;
          part.left = std::min(part.left, px);
          part.right = std::max(part.right, px);
          part.top = std::min(part.top, py);
          part.bottom = std::max(part.bottom, py);
          for (int dy = -1; dy <= 1; ++dy)
            for (int dx = -1; dx <= 1; ++dx) {
              int nx = px + dx, ny = py + dy;
              if (nx < 0 || nx >= value.width || ny < 0 || ny >= value.height)
                continue;
              int ni = ny * value.width + nx;
              if (labels[ni] < 0 && pixels[ni * 4 + 3] > 32) {
                labels[ni] = label;
                queue.push_back(ni);
              }
            }
        }
        parts.push_back(part);
        ++label;
      }
    std::sort(parts.begin(), parts.end(),
              [](const Part &a, const Part &b) { return a.count > b.count; });
    if (int(parts.size()) < columns * rows) {
      stbi_image_free(pixels);
      SDL_DestroyTexture(value.texture);
      throw std::runtime_error("Incomplete running atlas: " + path);
    }
    parts.resize(columns * rows);
    std::vector<bool> keep(label, false);
    for (const auto &p : parts)
      keep[p.label] = true;
    for (int i = 0; i < value.width * value.height; ++i)
      if (labels[i] < 0 || !keep[labels[i]])
        pixels[i * 4 + 3] = 0;
    // Sort by row bands, then by horizontal center. Whole connected bodies are
    // extracted without slicing a wide running foot at a nominal grid edge.
    std::sort(parts.begin(), parts.end(), [](const Part &a, const Part &b) {
      return a.top + a.bottom < b.top + b.bottom;
    });
    for (int row = 0; row < rows; ++row) {
      auto first = parts.begin() + row * columns, last = first + columns;
      std::sort(first, last, [](const Part &a, const Part &b) {
        return a.left + a.right < b.left + b.right;
      });
      float reference = 0;
      for (auto it = first; it != last; ++it)
        reference = std::max(reference, float(it->bottom - it->top + 1));
      for (auto it = first; it != last; ++it) {
        value.frames.push_back({float(it->left), float(it->top),
                                float(it->right - it->left + 1),
                                float(it->bottom - it->top + 1)});
        value.registration.push_back(reference);
        double sum = 0, weight = 0;
        int headBottom = it->top + int((it->bottom - it->top + 1) * .34f);
        for (int y = it->top; y < headBottom; ++y)
          for (int x = it->left; x <= it->right; ++x) {
            int at = y * value.width + x;
            if (labels[at] == it->label) {
              float a = pixels[at * 4 + 3] / 255.f;
              sum += x * a;
              weight += a;
            }
          }
        value.anchors.push_back(weight > 0 ? float(sum / weight)
                                           : float(it->left + it->right) * .5f);
      }
    }
  }
  if (columns > 1 && !components)
    for (int row = 0; row < rows; ++row)
      for (int col = 0; col < columns; ++col) {
        int x0 = col * value.width / columns,
            x1 = (col + 1) * value.width / columns,
            y0 = row * value.height / rows,
            y1 = (row + 1) * value.height / rows;
        // Retain the largest connected alpha component inside each cell.
        // Neighboring shoes and isolated marks must never become hat pixels.
        int cw = x1 - x0, ch = y1 - y0;
        std::vector<int> labels(cw * ch, -1), queue;
        std::vector<int> largest;
        int component = 0;
        for (int sy = 0; sy < ch; ++sy)
          for (int sx = 0; sx < cw; ++sx) {
            int start = sy * cw + sx;
            if (labels[start] >= 0 ||
                pixels[((y0 + sy) * value.width + x0 + sx) * 4 + 3] <= 32)
              continue;
            queue.clear();
            queue.push_back(start);
            labels[start] = component;
            for (size_t q = 0; q < queue.size(); ++q) {
              int at = queue[q], x = at % cw, y = at / cw;
              for (int dy = -1; dy <= 1; ++dy)
                for (int dx = -1; dx <= 1; ++dx) {
                  int nx = x + dx, ny = y + dy;
                  if (nx < 0 || nx >= cw || ny < 0 || ny >= ch)
                    continue;
                  int ni = ny * cw + nx;
                  if (labels[ni] < 0 &&
                      pixels[((y0 + ny) * value.width + x0 + nx) * 4 + 3] >
                          32) {
                    labels[ni] = component;
                    queue.push_back(ni);
                  }
                }
            }
            if (queue.size() > largest.size())
              largest = queue;
            ++component;
          }
        int keep = largest.empty() ? -2 : labels[largest.front()];
        int left = x1, right = x0, top = y1, bottom = y0;
        for (int y = 0; y < ch; ++y)
          for (int x = 0; x < cw; ++x) {
            auto &alpha = pixels[((y0 + y) * value.width + x0 + x) * 4 + 3];
            if (labels[y * cw + x] != keep) {
              alpha = 0;
              continue;
            }
            if (alpha > 32) {
              left = std::min(left, x0 + x);
              right = std::max(right, x0 + x);
              top = std::min(top, y0 + y);
              bottom = std::max(bottom, y0 + y);
            }
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
  bool uploaded =
      SDL_UpdateTexture(value.texture, nullptr, pixels, value.width * 4);
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
  traffic = image(assets + "/traffic.png", 4, 2);
  poses = image(assets + "/poses.png", 4, 3);
  carryPoses = image(assets + "/carry-poses.png", 4, 2);
  courierRun = image(assets + "/courier.png", 8, 4, true);
  carryRun = image(assets + "/carry-walk.png", 8, 4, true);
  runBetween = image(assets + "/run-inbetweens.png", 4, 4, true);
  carryBetween = image(assets + "/carry-inbetweens.png", 4, 4, true);
  thiefRun = image(assets + "/thief-run.png", 4, 4, true);
  for (int state = 0; state < 2; ++state) {
    float ref = 0;
    for (int i = 0; i < 8; ++i)
      ref = std::max(ref, thiefRun.frames[state * 8 + i].h);
    for (int i = 0; i < 8; ++i)
      thiefRun.registration[state * 8 + i] = ref;
  }
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
  SDL_DestroyTexture(traffic.texture);
  SDL_DestroyTexture(poses.texture);
  SDL_DestroyTexture(carryPoses.texture);
  SDL_DestroyTexture(courierRun.texture);
  SDL_DestroyTexture(carryRun.texture);
  SDL_DestroyTexture(runBetween.texture);
  SDL_DestroyTexture(carryBetween.texture);
  SDL_DestroyTexture(thiefRun.texture);
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
void Renderer::sprite(const Image &atlas, int index, Vec feet, float height,
                      float alpha, bool flip, bool registered) {
  const auto &src = atlas.frames[index];
  float scale = height / (registered ? atlas.registration[index] : src.h);
  float w = src.w * scale, h = src.h * scale;
  ellipse({feet.x, feet.y + 3}, height * .22f, height * .065f,
          {17, 43, 50, Uint8(75 * alpha)});
  SDL_SetTextureAlphaMod(atlas.texture, Uint8(255 * alpha));
  float anchor = registered ? (atlas.anchors[index] - src.x) : src.w * .5f;
  if (flip)
    anchor = src.w - anchor;
  SDL_FRect target{feet.x - anchor * scale, feet.y - h, w, h};
  checked(SDL_RenderTextureRotated(renderer, atlas.texture, &src, &target, 0,
                                   nullptr,
                                   flip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE));
}
void Renderer::courierSprite(Vec feet, int facing, float frame, float height,
                             float alpha, int pose, bool holding,
                             float stride) {
  if (pose >= 0) {
    sprite(holding ? carryPoses : poses, pose, feet, height, alpha);
    return;
  }
  (void)stride;
  // Original broad contacts plus four newly drawn recoil/passing frames.
  // Twelve chronological key drawings per direction, without mesh distortion.
  int phase = int(frame * 12) % 12;
  const int sequence[12] = {0, 1, -1, -2, 2, 3, 4, 5, -3, -4, 6, 7};
  int key = sequence[phase];
  bool flip = (facing == 0 || facing == 2) && phase >= 6;
  if (key >= 0)
    sprite(holding ? carryRun : courierRun, facing * 8 + key, feet, height,
           alpha, flip, true);
  else
    sprite(holding ? carryBetween : runBetween, facing * 4 + (-key - 1), feet,
           height, alpha, flip, true);
}
void Renderer::balloon(Vec anchor, const std::string &title,
                       const std::string &detail, bool interact) {
  float w = std::max(textWidth(title, 29), textWidth(detail, 21)) +
            (interact ? 105.f : 58.f);
  w = std::clamp(w, 245.f, 510.f);
  float x = std::clamp(anchor.x - w / 2, 18.f, Width - w - 18);
  float y = std::max(130.f, anchor.y - 102);
  round({x + 2, y + 7, w, 85}, 24, {7, 29, 36, 75});
  round({x, y, w, 85}, 24, Ink);
  round({x + 3, y + 3, w - 6, 79}, 21, Cream);
  float tail = std::clamp(anchor.x, x + 32, x + w - 32);
  SDL_Vertex outline[] = {{{tail - 14, y + 82},
                           {Ink.r / 255.f, Ink.g / 255.f, Ink.b / 255.f, 1},
                           {0, 0}},
                          {{tail + 14, y + 82},
                           {Ink.r / 255.f, Ink.g / 255.f, Ink.b / 255.f, 1},
                           {0, 0}},
                          {{tail, y + 101},
                           {Ink.r / 255.f, Ink.g / 255.f, Ink.b / 255.f, 1},
                           {0, 0}}};
  SDL_RenderGeometry(renderer, nullptr, outline, 3, nullptr, 0);
  for (auto &v : outline)
    v.color = {Cream.r / 255.f, Cream.g / 255.f, Cream.b / 255.f, 1};
  outline[0].position = {tail - 9, y + 80};
  outline[1].position = {tail + 9, y + 80};
  outline[2].position = {tail, y + 95};
  SDL_RenderGeometry(renderer, nullptr, outline, 3, nullptr, 0);
  float center = x + w / 2 + (interact ? 26.f : 0.f);
  if (interact) {
    round({x + 14, y + 22, 53, 39}, 10, Ink);
    text("SPACE", x + 40, y + 32, 15, Cream, true);
  }
  text(title, center, y + 12, 29, Ink, true);
  text(detail, center, y + 49, 21, {51, 100, 98}, true);
}
void Renderer::vehicle(const Car &car, float time) {
  const auto &src = traffic.frames[car.style + (car.speed < 0 ? 4 : 0)];
  float w = car.width + 26, h = std::min(146.f, w * src.h / src.w);
  ellipse({car.x, car.y + 36}, w * .45f, 12, {9, 27, 35, 70});
  picture(traffic,
          {car.x - w * .5f,
           car.y + 44 - h + std::sin(time * 8 + car.style * 1.7f) * .7f, w, h},
          &src);
  if (car.signal > 0 || car.mergeAge > 0) {
    float side = car.mergeTo > car.mergeFrom ? 1.f : -1.f;
    float blink = std::sin(time * 16) > 0 ? 1.f : .25f;
    ellipse({car.x + w * .32f, car.y + 16 + side * 19}, 7, 5,
            {255, 190, 55, Uint8(255 * blink)});
    if (car.signal > 0) {
      for (int i = 1; i < 6; ++i)
        ellipse({car.x + i * 12.f, car.y + 22 + side * i * 20}, 4, 4,
                {255, 211, 107, 150});
    }
  }
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
    if (!game.canInteract())
      balloon({destination.x, destination.y - 25},
              game.run.carrying ? "DELIVER HERE" : "PIZZA PICKUP",
              game.run.carrying ? "One hot pizza, one happy customer"
                                : "Fresh from the oven");
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
    if (game.run.thief.active && game.run.thief.warning <= 0)
      order.push_back({game.run.thief.p.y, -2});
    std::sort(order.begin(), order.end(),
              [](DrawItem a, DrawItem b) { return a.y < b.y; });
    for (auto item : order)
      if (item.index == -2) {
        const auto &t = game.run.thief;
        Vec p = t.previous * (1 - blend) + t.p * blend;
        sprite(thiefRun, (t.hasPizza ? 8 : 0) + int(t.animation * 8) % 8, p,
               121, 1, t.direction < 0, true);
      } else if (item.index < 0) {
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
                      pose, holding,
                      std::min(1.f, length(game.run.velocity) / 160.f));
      } else {
        Car car = game.run.cars[item.index];
        car.x -= car.speed * ((1 - blend) / 120.f);
        if (car.previousY > 0)
          car.y = car.previousY * (1 - blend) + car.y * blend;
        vehicle(car, time);
      }
    for (const auto &p : game.particles)
      ellipse(
          p.p, p.size, p.size,
          rgb(p.color, Uint8(255 * std::clamp(p.life / p.total, 0.f, 1.f))));
    for (const auto &f : game.floaters) {
      if (game.canInteract())
        continue;
      float tw = std::min(1100.f, std::max(520.f, textWidth(f.text, 26) + 54));
      round({f.p.x - tw / 2, f.p.y - 3, tw, 43}, 16,
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
    Vec feet = r.previous * (1 - blend) + r.player * blend;
    balloon(
        {feet.x, feet.y - 124}, r.carrying ? "DELIVER PIZZA" : "GRAB A PIZZA",
        r.carrying ? "Cash in that hot delivery!" : "A fresh order is waiting",
        true);
  }
  text("TRAFFIC " + std::to_string(r.level), 366, 99, 15, Mint);
  if (r.thief.active && r.thief.warning > 0)
    balloon({r.thief.direction > 0 ? 160.f : 1760.f, r.thief.p.y - 65},
            "PIZZA THIEF!", "Keep your pizza out of reach");
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
    text("Watch amber signals: cars change lanes as traffic levels rise.", 960,
         655, 24, Gold, true);
    text("Keep your pizza away from the thief. Dash to create space!", 960, 688,
         24, Cream, true);
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
