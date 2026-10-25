// SPDX-License-Identifier: GPL-2.0-or-later
#include "audio.hpp"
#include "game.hpp"
#include "render.hpp"
#include <SDL3/SDL_main.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <string>
namespace xfrog {
static std::string profilePath() {
  char *base = SDL_GetPrefPath("Gustavo Muller", "Xfrog 2");
  if (!base)
    throw std::runtime_error(SDL_GetError());
  std::string path = std::string(base) + "profile.dat";
  SDL_free(base);
  return path;
}
} // namespace xfrog
int main(int argc, char **argv) {
  using namespace xfrog;
  bool smoke = false, full = false;
  uint32_t seed = uint32_t(SDL_GetPerformanceCounter());
  for (int i = 1; i < argc; ++i) {
    if (!std::strcmp(argv[i], "--smoke-test"))
      smoke = true;
    else if (!std::strcmp(argv[i], "--fullscreen"))
      full = true;
    else if (!std::strcmp(argv[i], "--seed") && i + 1 < argc)
      seed = uint32_t(std::strtoul(argv[++i], nullptr, 10));
    else if (!std::strcmp(argv[i], "--version")) {
      std::printf("Xfrog %s\n", Version);
      return 0;
    } else if (!std::strcmp(argv[i], "--help")) {
      puts("Xfrog 2.0: arrows/WASD move, Space interact, Shift dash, Escape "
           "pause, H help, M sound, F11 fullscreen.\nOptions: --fullscreen "
           "--seed N --smoke-test --version");
      return 0;
    } else {
      std::fprintf(stderr, "Unknown option: %s\n", argv[i]);
      return 1;
    }
  }
  if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMEPAD)) {
    std::fprintf(stderr, "SDL: %s\n", SDL_GetError());
    return 1;
  }
  int status = 0;
  try {
    const char *base = SDL_GetBasePath();
    if (!base)
      throw std::runtime_error(SDL_GetError());
    Game game(seed);
    std::string save = profilePath();
    game.profile = loadProfile(save);
    Renderer renderer(std::string(base) + "assets/v2");
    Audio audio;
    if (full)
      renderer.fullscreen();
    bool quit = false, focused = true;
    Vec mouse{-1, -1};
    Controls input;
    SDL_Gamepad *pad = nullptr;
    int count = 0;
    SDL_JoystickID *ids = SDL_GetGamepads(&count);
    if (count)
      pad = SDL_OpenGamepad(ids[0]);
    SDL_free(ids);
    auto action = [&](const std::string &command) {
      if (command == "start") {
        game.start();
        input = {};
      } else if (command == "resume")
        game.resume();
      else if (command == "menu") {
        game.menu();
        saveProfile(save, game.profile);
      } else if (command == "next")
        game.start(game.run.won);
      else if (command == "help")
        game.help();
      else if (command == "closehelp")
        game.closeHelp();
      else if (command == "sound")
        game.profile.sound = !game.profile.sound;
      else if (command == "relaxed")
        game.profile.difficulty = Difficulty::Relaxed;
      else if (command == "arcade")
        game.profile.difficulty = Difficulty::Arcade;
      if (!command.empty())
        game.cues.push_back(Cue::Click);
    };
    Uint64 previous = SDL_GetTicksNS(), next = previous;
    double accumulator = 0;
    float elapsed = 0;
    unsigned frames = 0;
    Uint64 began = previous;
    while (!quit) {
      SDL_Event event;
      while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT ||
            event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED)
          quit = true;
        else if (event.type == SDL_EVENT_WINDOW_FOCUS_LOST) {
          focused = false;
          game.pause();
          input = {};
          accumulator = 0;
        } else if (event.type == SDL_EVENT_WINDOW_FOCUS_GAINED) {
          focused = true;
          previous = SDL_GetTicksNS();
          next = previous;
        } else if (event.type == SDL_EVENT_MOUSE_MOTION)
          mouse = renderer.mousePosition(event.motion.x, event.motion.y);
        else if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN &&
                 event.button.button == SDL_BUTTON_LEFT) {
          mouse = renderer.mousePosition(event.button.x, event.button.y);
          action(renderer.clicked(mouse));
        } else if (event.type == SDL_EVENT_GAMEPAD_ADDED && !pad)
          pad = SDL_OpenGamepad(event.gdevice.which);
        else if (event.type == SDL_EVENT_GAMEPAD_REMOVED && pad &&
                 SDL_GetGamepadID(pad) == event.gdevice.which) {
          SDL_CloseGamepad(pad);
          pad = nullptr;
        } else if (event.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN) {
          if (event.gbutton.button == SDL_GAMEPAD_BUTTON_SOUTH) {
            if (game.screen == Screen::Menu)
              action("start");
            else
              input.interact = true;
          } else if (event.gbutton.button == SDL_GAMEPAD_BUTTON_EAST)
            input.dash = true;
          else if (event.gbutton.button == SDL_GAMEPAD_BUTTON_START) {
            if (game.screen == Screen::Playing)
              game.pause();
            else
              game.resume();
          }
        } else if (event.type == SDL_EVENT_KEY_DOWN && !event.key.repeat) {
          auto key = event.key.scancode;
          if (key == SDL_SCANCODE_F11 ||
              (key == SDL_SCANCODE_RETURN && (event.key.mod & SDL_KMOD_ALT)))
            renderer.fullscreen();
          else if (key == SDL_SCANCODE_ESCAPE || key == SDL_SCANCODE_P) {
            if (game.screen == Screen::Playing)
              game.pause();
            else if (game.screen == Screen::Paused)
              game.resume();
            else if (game.screen == Screen::Help)
              game.closeHelp();
            else if (game.screen == Screen::Results)
              action("menu");
            else if (key == SDL_SCANCODE_ESCAPE)
              quit = true;
            input = {};
          } else if (key == SDL_SCANCODE_H) {
            if (game.screen == Screen::Help)
              game.closeHelp();
            else
              game.help();
          } else if (key == SDL_SCANCODE_M)
            game.profile.sound = !game.profile.sound;
          else if (key == SDL_SCANCODE_RETURN) {
            if (game.screen == Screen::Menu)
              action("start");
            else if (game.screen == Screen::Paused)
              action("resume");
            else if (game.screen == Screen::Results)
              action("next");
            else if (game.screen == Screen::Help)
              game.closeHelp();
          } else if (game.screen == Screen::Menu &&
                     (key == SDL_SCANCODE_LEFT || key == SDL_SCANCODE_RIGHT))
            game.profile.difficulty = key == SDL_SCANCODE_LEFT
                                          ? Difficulty::Relaxed
                                          : Difficulty::Arcade;
          else if (key == SDL_SCANCODE_SPACE)
            input.interact = true;
          else if (key == SDL_SCANCODE_LSHIFT || key == SDL_SCANCODE_RSHIFT)
            input.dash = true;
        }
      }
      if (quit)
        break;
      Uint64 now = SDL_GetTicksNS();
      double delta = std::min(double(now - previous) / 1e9, .1);
      previous = now;
      elapsed += float(delta);
      if (!focused && !smoke) {
        audio.update(false, true, {});
        SDL_Delay(20);
        continue;
      }
      const bool *keys = SDL_GetKeyboardState(nullptr);
      input.move = {float(keys[SDL_SCANCODE_D] || keys[SDL_SCANCODE_RIGHT]) -
                        float(keys[SDL_SCANCODE_A] || keys[SDL_SCANCODE_LEFT]),
                    float(keys[SDL_SCANCODE_S] || keys[SDL_SCANCODE_DOWN]) -
                        float(keys[SDL_SCANCODE_W] || keys[SDL_SCANCODE_UP])};
      if (pad) {
        float x = SDL_GetGamepadAxis(pad, SDL_GAMEPAD_AXIS_LEFTX) / 32767.f,
              y = SDL_GetGamepadAxis(pad, SDL_GAMEPAD_AXIS_LEFTY) / 32767.f;
        if (std::abs(x) > .2f || std::abs(y) > .2f)
          input.move = {x, y};
      }
      accumulator += delta;
      Screen before = game.screen;
      while (accumulator >= 1.0 / 120) {
        game.update(1.f / 120, input);
        input.interact = false;
        input.dash = false;
        accumulator -= 1.0 / 120;
      }
      if (before == Screen::Playing && game.screen == Screen::Results &&
          !saveProfile(save, game.profile))
        SDL_Log("Could not save profile");
      audio.update(game.profile.sound,
                   game.screen == Screen::Paused || game.screen == Screen::Help,
                   game.cues);
      game.cues.clear();
      renderer.draw(game, elapsed, mouse);
      ++frames;
      if (smoke && frames >= 120) {
        const char *capture = SDL_getenv("XFROG_SCREENSHOT");
        if (capture)
          renderer.capture(capture);
        std::printf("PASS: Xfrog 2.0 rendered 120 HD frames (%.1f fps)\n",
                    frames * 1e9 / double(SDL_GetTicksNS() - began));
        break;
      }
      next += 1000000000 / 60;
      now = SDL_GetTicksNS();
      if (next > now)
        SDL_DelayNS(next - now);
      else
        next = now;
    }
    if (pad)
      SDL_CloseGamepad(pad);
    if (!saveProfile(save, game.profile))
      SDL_Log("Could not save profile");
    puts("Xfrog 2.0 shut down cleanly");
  } catch (const std::exception &error) {
    std::fprintf(stderr, "Xfrog: %s\n", error.what());
    status = 1;
  }
  SDL_Quit();
  return status;
}
