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
    for (int holding = 0; holding < 2; ++holding) {
      game.run.carrying = holding != 0;
      game.run.velocity = {285, 0};
      for (int facing = 0; facing < 4; ++facing) {
        std::set<uint64_t> poses;
        game.run.facing = facing;
        for (int frame = 0; frame < RunFrames; ++frame) {
          game.run.animation = frame / float(RunFrames);
          renderer.draw(game, 1, {-1, -1});
          SDL_Rect region{880, 875, 160, 160};
          std::string path =
              std::string(argv[2]) + "/walk-" + std::to_string(holding) + "-" +
              std::to_string(facing) + "-" + std::to_string(frame) + ".bmp";
          renderer.capture(path, &region);
          SDL_Surface *crop = SDL_LoadBMP(path.c_str());
          check(crop && crop->w == 160 && crop->h == 160, "courier frame crop");
          uint64_t hash = 1469598103934665603ull;
          for (int y = 110; y < 150; ++y)
            for (int x = 45; x < 115; ++x) {
              Uint8 r, g, b, a;
              SDL_ReadSurfacePixel(crop, x, y, &r, &g, &b, &a);
              hash = (hash ^ (uint32_t(r) << 16 | uint32_t(g) << 8 | b)) *
                     1099511628211ull;
            }
          poses.insert(hash);
          SDL_DestroySurface(crop);
        }
        check(poses.size() >= 5,
              "running has distinct sprite poses in every direction");
      }
    }
    game.run.carrying = false;
    game.run.velocity = {};
    game.run.facing = 0;
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
        game.run.animation = frame / 8.f;
        renderer.draw(game, 1, {-1, -1}, .5f);
      }
    }
    game.run.velocity = {};
    capture("order");
    game.run.thief.active = true;
    game.run.thief.warning = 0;
    game.run.thief.p = game.run.thief.previous = {510, DeliverySidewalkY};
    capture("thief");
    for (int holding = 0; holding < 2; ++holding)
      for (int direction = 0; direction < 2; ++direction) {
        game.run.thief.hasPizza = holding != 0;
        game.run.thief.direction = direction == 0 ? 1.f : -1.f;
        for (int frame = 0; frame < 8; ++frame) {
          game.run.thief.animation = frame / 8.f;
          renderer.draw(game, 1, {-1, -1});
          SDL_Rect region{420, 205, 180, 165};
          std::string path = std::string(argv[2]) + "/thief-run-" +
                             std::to_string(holding) + "-" +
                             std::to_string(direction) + "-" +
                             std::to_string(frame) + ".bmp";
          renderer.capture(path, &region);
        }
      }
    game.run.thief.animation = 0;
    game.run.thief.direction = 1;
    game.run.thief.hasPizza = true;
    capture("thief-stolen-pizza");
    game.run.thief.active = false;
    game.run.cars[0].x = 500;
    game.run.cars[0].signal = 1;
    game.run.cars[0].mergeFrom = 416;
    game.run.cars[0].mergeTo = 616;
    capture("lane-change-warning");
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
    // Actual model ticks played at 30 Hz, not hand-picked still poses.
    for (int holding = 0; holding < 2; ++holding) {
      Game playback(7);
      playback.start();
      playback.run.cars.clear();
      playback.run.player = playback.run.previous = {100, 350};
      playback.run.velocity = {285, 0};
      playback.run.carrying = holding != 0;
      playback.run.thief.wait = 1000;
      int last = runFrame(playback.run.animation);
      for (int frame = 0; frame < 150; ++frame) {
        for (int tick = 0; tick < 4; ++tick)
          playback.update(1.f / 120, {{1, 0}, false, false});
        int current = runFrame(playback.run.animation);
        check((current - last + RunFrames) % RunFrames <= 1,
              "rendered playback never skips a drawing");
        last = current;
        renderer.draw(playback, frame / 30.f, {-1, -1});
        SDL_Rect region{int(playback.run.player.x) - 80, 215, 160, 155};
        std::string path = std::string(argv[2]) + "/playback-" +
                           std::to_string(holding) + "-" +
                           std::to_string(frame) + ".bmp";
        renderer.capture(path, &region);
      }
    }
    check(Audio::loadMusic(std::string(argv[1]) + "/town-theme.wav"),
          "stereo town score loads");
    double energy = 0;
    float peak = 0;
    for (uint64_t i = 0; i < 48000; ++i) {
      float sample = Audio::musicSample(i);
      check(std::isfinite(sample), "music sample is finite");
      energy += sample * sample;
      peak = std::max(peak, std::abs(sample));
    }
    check(energy > 1 && peak < .3f,
          "original music has audible bounded output");
    Audio audio;
    audio.update(true, false, {Cue::Pickup, Cue::Deliver, Cue::Win});
    audio.update(false, false, {});
    audio.update(true, true, {});
    puts("PASS: HD scenes, continuous walks and restored thief, true-color "
         "rendering, "
         "sprite atlases, UI hit "
         "targets and bounded original audio");
  } catch (const std::exception &e) {
    fprintf(stderr, "FAIL: %s\n", e.what());
    result = 1;
  }
  SDL_Quit();
  return result;
}
