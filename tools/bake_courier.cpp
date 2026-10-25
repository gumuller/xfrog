// SPDX-License-Identifier: GPL-2.0-or-later
// Offline only. The shipped game loads complete PNG sprites, never paints
// limbs.
#include "render.hpp"
#define SDL_MAIN_HANDLED
#include <SDL3/SDL_main.h>
#include <cstdio>
#include <filesystem>
#include <stdexcept>
namespace xfrog {
static void checked(bool ok) {
  if (!ok)
    throw std::runtime_error(SDL_GetError());
}
void Renderer::paintCourier(Vec feet, int facing, float cycle, float height,
                            float alpha, bool holding, float activity) {
  const auto &atlas = courierParts[facing == 3 ? 1 : std::clamp(facing, 0, 2)];
  float mirror = facing == 3 ? -1.f : 1.f;
  bool side = facing == 1 || facing == 3;
  auto pose = courierPose(cycle, facing, activity);
  auto part = [&](int index, RigPoint pivot, float width, float h, float ax,
                  float ay, float angle, float shade) {
    if (mirror < 0)
      ax = 1 - ax;
    SDL_FRect dst{feet.x + mirror * pivot.x * height - ax * width * height,
                  feet.y + pivot.y * height - ay * h * height, width * height,
                  h * height};
    SDL_FPoint center{ax * dst.w, ay * dst.h};
    SDL_SetTextureColorMod(atlas.texture, Uint8(255 * shade),
                           Uint8(255 * shade), Uint8(255 * shade));
    SDL_SetTextureAlphaMod(atlas.texture, Uint8(255 * alpha));
    checked(SDL_RenderTextureRotated(
        renderer, atlas.texture, &atlas.frames[index], &dst, angle * mirror,
        &center, mirror < 0 ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE));
  };
  auto limb = [&](int index, RigPoint from, RigPoint to, float width,
                  float shade) {
    float dx = to.x - from.x, dy = to.y - from.y;
    part(index, from, width, std::hypot(dx, dy) / .76f, .5f, .12f,
         std::atan2(-dx, dy) * 180 / AnimationPi, shade);
  };
  for (int leg : {1, 0}) {
    const auto &p = pose.legs[leg];
    float shade = leg == 1 ? .84f : 1.f;
    limb(5, p.knee, p.ankle, .115f, shade);
    limb(4, p.hip, p.knee, .14f, shade);
    part(6 + leg, p.ankle, side ? .25f : .17f, side ? .13f : .19f,
         side ? .25f : .5f, side ? .435f : .302f, p.footAngle, shade);
  }
  if (!holding) {
    for (int arm : {1, 0})
      limb(2, pose.shoulders[arm], pose.elbows[arm], .125f,
           arm == 1 ? .84f : 1.f);
    limb(3, pose.elbows[1], pose.hands[1], .12f, .84f);
  }
  const auto &body = atlas.frames[holding ? 1 : 0];
  float h = .60f / .94f;
  part(holding ? 1 : 0, {0, pose.hipY}, h * body.w / body.h, h,
       side ? (holding ? .44f : .54f) : .5f, .94f, pose.lean, 1);
  if (!holding)
    limb(3, pose.elbows[0], pose.hands[0], .12f, 1);
  SDL_SetTextureColorMod(atlas.texture, 255, 255, 255);
}
void Renderer::bakeCourier(const std::string &output) {
  std::filesystem::create_directories(output);
  const char *directions[] = {"front", "side", "back"};
  SDL_Texture *target = SDL_CreateTexture(
      renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET,
      CourierColumns * CourierTile, CourierRows * CourierTile);
  if (!target)
    throw std::runtime_error(SDL_GetError());
  SDL_SetRenderTarget(renderer, target);
  SDL_SetRenderLogicalPresentation(renderer, 0, 0,
                                   SDL_LOGICAL_PRESENTATION_DISABLED);
  for (int holding = 0; holding < 2; ++holding)
    for (int facing = 0; facing < 3; ++facing) {
      SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
      SDL_RenderClear(renderer);
      for (int index = 0; index < CourierSpriteCount; ++index) {
        int level = index == 0 ? 0 : (index - 1) / CourierSamples + 1;
        float phase =
            index == 0 ? 0
                       : ((index - 1) % CourierSamples) / float(CourierSamples);
        Vec feet{index % CourierColumns * float(CourierTile) + CourierAnchorX,
                 index / CourierColumns * float(CourierTile) + CourierAnchorY};
        paintCourier(feet, facing, phase, CourierBakeHeight, 1, holding,
                     level / float(CourierActivitySteps));
      }
      SDL_Surface *readback = SDL_RenderReadPixels(renderer, nullptr);
      if (!readback)
        throw std::runtime_error(SDL_GetError());
      SDL_Surface *surface =
          SDL_ConvertSurface(readback, SDL_PIXELFORMAT_RGBA32);
      SDL_DestroySurface(readback);
      if (!surface)
        throw std::runtime_error(SDL_GetError());
      // Drawing over transparency stores premultiplied RGB. PNGs use straight
      // alpha; convert once offline so silhouettes retain clean colored edges.
      for (int y = 0; y < surface->h; ++y) {
        auto *row = static_cast<Uint8 *>(surface->pixels) + y * surface->pitch;
        for (int x = 0; x < surface->w; ++x) {
          auto *pixel = row + x * 4;
          if (pixel[3])
            for (int c = 0; c < 3; ++c)
              pixel[c] = Uint8(std::min(
                  255, (int(pixel[c]) * 255 + pixel[3] / 2) / pixel[3]));
        }
      }
      for (int index = 0; index < CourierSpriteCount; ++index) {
        int left = index % CourierColumns * CourierTile,
            top = index / CourierColumns * CourierTile, visible = 0;
        for (int y = 0; y < CourierTile; ++y) {
          auto *row = static_cast<Uint8 *>(surface->pixels) +
                      (top + y) * surface->pitch;
          for (int x = 0; x < CourierTile; ++x) {
            if (row[(left + x) * 4 + 3] <= 16)
              continue;
            ++visible;
            if (x < 2 || y < 2 || x >= CourierTile - 2 || y >= CourierTile - 2)
              throw std::runtime_error(
                  "A baked hand or foot touches the cell boundary");
          }
        }
        if (visible < 1200)
          throw std::runtime_error("Incomplete baked courier sprite");
      }
      std::string path = output + "/courier-baked-" + std::to_string(holding) +
                         "-" + directions[facing] + ".png";
      bool saved = SDL_SavePNG(surface, path.c_str());
      SDL_DestroySurface(surface);
      if (!saved)
        throw std::runtime_error(SDL_GetError());
      printf("Baked %d full sprites: %s\n", CourierSpriteCount, path.c_str());
    }
  SDL_SetRenderTarget(renderer, nullptr);
  SDL_DestroyTexture(target);
}
} // namespace xfrog
int main(int argc, char **argv) {
  SDL_SetMainReady();
  if (argc != 3 || !SDL_Init(SDL_INIT_VIDEO))
    return 1;
  int result = 0;
  try {
    xfrog::Renderer renderer(argv[1], true, true);
    renderer.setVsync(false);
    renderer.bakeCourier(argv[2]);
  } catch (const std::exception &e) {
    fprintf(stderr, "%s\n", e.what());
    result = 1;
  }
  SDL_Quit();
  return result;
}
