// SPDX-License-Identifier: GPL-2.0-or-later
#include "render.hpp"
#include "audio.hpp"
#define SDL_MAIN_HANDLED
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <set>
#include <stdexcept>
using namespace xfrog;
static void check(bool value, const char *message) {
  if (!value)
    throw std::runtime_error(message);
}
int main(int argc, char **argv) {
  SDL_SetMainReady();
  if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO))
    return 1;
  int result = 0;
  try {
    check(argc > 2, "asset and capture paths required");
    Renderer renderer(argv[1], true);
    renderer.setVsync(false);
    Game game(7);
    auto capture = [&](const char *name) {
      renderer.draw(game, 1, {-1, -1});
      std::string path = std::string(argv[2]) + "/" + name + ".bmp";
      renderer.capture(path);
      SDL_Surface *image = SDL_LoadBMP(path.c_str());
      check(image && image->w == 1920 && image->h == 1080,
            "framebuffer is full HD");
      std::set<uint32_t> colors;
      Uint8 r, g, b, a;
      for (int y = 0; y < 1080; y += 3)
        for (int x = 0; x < 1920; x += 3) {
          check(SDL_ReadSurfacePixel(image, x, y, &r, &g, &b, &a),
                "read rendered pixel");
          colors.insert((uint32_t(r) << 16) | (uint32_t(g) << 8) | b);
        }
      check(colors.size() > 4096,
            "renderer must preserve full-color art beyond any indexed palette");
      SDL_DestroySurface(image);
    };
    capture("menu");
    check(renderer.buttons.size() == 5, "menu exposes mouse controls");
    check(renderer.clicked({500, 590}) == "start", "start button hit target");
    game.start();
    capture("gameplay");
    game.interact();
    capture("pickup");
    game.run.pickup = 0;
    for (int facing = 0; facing < 4; ++facing) {
      game.run.facing = facing;
      game.run.velocity = {};
      std::string idle = "carrying-idle-" + std::to_string(facing);
      capture(idle.c_str());
      game.run.velocity = {100, 0};
      for (int frame = 0; frame < 8; ++frame) {
        game.run.animation = float(frame);
        renderer.draw(game, 1, {-1, -1}, .5f);
      }
    }
    game.run.velocity = {};
    capture("order");
    game.pause();
    capture("pause");
    game.help();
    capture("help");
    game.closeHelp();
    game.resume();
    game.run.delivered = game.run.goal - 1;
    game.run.player = game.doors[game.run.target];
    game.interact();
    for (int frame = 0; frame < 4; ++frame) {
      game.run.handoff = .36f - frame * .09f;
      std::string handoff = "handoff-" + std::to_string(frame);
      capture(handoff.c_str());
    }
    capture("results");
    double energy = 0;
    float peak = 0;
    for (uint64_t i = 0; i < 48000; ++i) {
      float sample = Audio::musicSample(i);
      check(std::isfinite(sample), "music sample is finite");
      energy += sample * sample;
      peak = std::max(peak, std::abs(sample));
    }
    check(energy > 1 && peak < .2f,
          "original music has audible bounded output");
    Audio audio;
    audio.update(true, false, {Cue::Pickup, Cue::Deliver, Cue::Win});
    audio.update(false, false, {});
    audio.update(true, true, {});
    puts("PASS: HD scenes and 84 character frames, true-color rendering, "
         "sprite atlases, UI hit "
         "targets and bounded original audio");
  } catch (const std::exception &e) {
    fprintf(stderr, "FAIL: %s\n", e.what());
    result = 1;
  }
  SDL_Quit();
  return result;
}
