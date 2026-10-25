// SPDX-License-Identifier: GPL-2.0-or-later
#include "game.hpp"
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <limits>
#include <stdexcept>
using namespace xfrog;
static int assertions = 0;
static void check(bool ok, const char *text) {
  ++assertions;
  if (!ok)
    throw std::runtime_error(text);
}
static void ticks(Game &g, int count, Controls c = {}) {
  for (int i = 0; i < count; ++i)
    g.update(1.f / 120, c);
}
static void model() {
  Game g(7);
  g.start();
  g.run.cars.clear();
  check(g.run.hearts == 5 && g.run.time == 180,
        "relaxed has accessible defaults");
  check(g.canInteract(), "spawn is in pickup range");
  g.interact();
  check(g.run.carrying && g.run.heat == 1, "pickup assigns fresh pizza");
  check(g.run.pickup > 0, "pickup starts visible box collection");
  ticks(g, 40);
  check(g.run.pickup == 0 && g.run.carrying,
        "pickup settles into holding idle");
  g.run.heat = 1;
  int target = g.run.target;
  g.run.player = g.doors[target];
  g.interact();
  check(!g.run.carrying && g.run.score == 170 && g.run.combo == 1,
        "hot delivery scores base plus tip");
  check(g.run.handoff > 0 && g.run.pickup == 0, "delivery starts box handoff");
  g.run.player = g.shop;
  g.interact();
  check(g.run.target != target, "consecutive delivery targets differ");
  g.run.heat = .001f;
  ticks(g, 30);
  check(!g.run.carrying && g.run.combo == 0,
        "cold pizza drops order and resets streak");
  check(g.run.score == 145, "expired pizza applies bounded penalty");
  g.run.player = {650, 800};
  Vec before = g.run.player;
  ticks(g, 120, {{1, 0}, false, false});
  check(g.run.player.x < 680, "solid shop prevents walking through its roof");
  g.run.player = {400, 800};
  g.run.velocity = {};
  ticks(g, 120, {{1, 0}, false, false});
  float straight = g.run.player.x - 400;
  g.run.player = {400, 800};
  g.run.velocity = {};
  ticks(g, 120, {{1, 1}, false, false});
  float diagonal = length(g.run.player - Vec{400, 800});
  check(std::abs(straight - diagonal) < 2, "diagonal movement is normalized");
  g.run.player = {500, 800};
  g.run.velocity = {};
  g.run.dashCooldown = 0;
  g.update(1.f / 120, {{-1, 0}, false, true});
  check(g.run.dashTime > 0 && g.run.dashCooldown > 1,
        "dash starts with cooldown");
  ticks(g, 30, {{-1, 0}, false, true});
  check(g.run.dashTime == 0 && g.run.dashCooldown > 0,
        "holding dash cannot bypass cooldown");
  int hearts = g.run.hearts;
  g.hit();
  check(g.run.hearts == hearts - 1 && !g.run.carrying,
        "traffic hit costs one heart and drops pizza");
  g.hit();
  check(g.run.hearts == hearts - 1, "respawn grace prevents chained hits");
  float time = g.run.time;
  Vec position = g.run.player;
  g.pause();
  ticks(g, 120, {{1, 0}, true, true});
  check(g.run.time == time && length(g.run.player - position) == 0,
        "pause freezes movement, timer and interactions");
  g.resume();
  check(g.screen == Screen::Playing, "resume restores gameplay");
  g.help();
  time = g.run.time;
  ticks(g, 120);
  check(g.run.time == time, "help freezes shift");
  g.closeHelp();
  check(g.screen == Screen::Playing, "help returns to previous screen");
  for (int i = 0; i < g.run.goal; ++i) {
    g.run.player = g.shop;
    g.run.carrying = false;
    g.interact();
    g.run.player = g.doors[g.run.target];
    g.interact();
    if (g.screen == Screen::Results)
      break;
  }
  check(g.screen == Screen::Results && g.run.won,
        "delivery goal completes shift");
  check(g.profile.best >= g.run.score, "record updates on success");
  check(g.run.handoff > 0, "winning delivery retains its handoff");
  float resultTime = g.run.time;
  ticks(g, 120);
  check(g.run.handoff == 0 && g.run.celebration == 0 && g.particles.empty(),
        "results finish visual animation without freezing sprites");
  check(g.run.time == resultTime, "results freeze gameplay clock");
  int shift = g.run.shift;
  g.start(true);
  check(g.run.shift == shift + 1 && g.run.goal == 8,
        "next shift progresses goal and traffic");
  g.run.time = .001f;
  ticks(g, 1);
  check(g.screen == Screen::Results && !g.run.won,
        "shift timer ends incomplete run");
  g.profile.difficulty = Difficulty::Arcade;
  g.start();
  check(g.run.hearts == 3 && g.run.cars.size() == 12,
        "arcade has distinct challenge");
  time = g.run.time;
  g.update(std::numeric_limits<float>::quiet_NaN(), {});
  g.update(-1, {});
  check(g.run.time == time, "invalid frame delta cannot corrupt simulation");
  g.update(100, {});
  check(time - g.run.time <= .051f, "long stalls are bounded");
  check(!g.clearAt({-2, 100}) && !g.clearAt({900, 800}) &&
            g.clearAt({650, 800}),
        "world bounds and safe path agree with collision map");
  (void)before;
}
static void traffic() {
  Game g(7);
  g.start();
  g.run.cars = {{500, 416, 0, 158, 0, false}};
  g.run.player = {500, 438};
  int hearts = g.run.hearts;
  ticks(g, 1);
  check(g.run.hearts == hearts - 1 && length(g.run.player - g.shop) < 1,
        "bumper collision respawns courier");
  g.start();
  g.run.cars = {{500, 416, 0, 158, 0, false}};
  g.run.player = {500, 370};
  ticks(g, 1);
  check(g.run.hearts == 5, "painted car roof does not create unfair collision");
  g.run.player = {500, 500};
  g.run.cars[0].nearAwarded = false;
  g.run.nearMisses = 0;
  g.run.cars[0].x = 500;
  g.run.cars[0].y = 416;
  int score = g.run.score;
  ticks(g, 1);
  check(g.run.score == score + 5 && g.run.nearMisses == 1,
        "close call awards a small bonus");
  ticks(g, 10);
  check(g.run.score == score + 5,
        "same vehicle cannot repeatedly award near miss");
  g.run.cars[0].x = Width + 301;
  g.run.cars[0].speed = 1;
  ticks(g, 1);
  check(g.run.cars[0].x < 0 && !g.run.cars[0].nearAwarded,
        "wrapped traffic resets close call eligibility");
}
static void profile(const std::string &folder) {
  std::filesystem::create_directories(folder);
  std::string path = folder + "/profile-test.dat";
  std::remove(path.c_str());
  Profile p = loadProfile(path);
  check(p.best == 0 && p.sound, "missing profile defaults safely");
  p.best = 987;
  p.totalDeliveries = 12;
  p.sound = false;
  p.difficulty = Difficulty::Arcade;
  check(saveProfile(path, p), "profile writes atomically");
  auto loaded = loadProfile(path);
  check(loaded.best == 987 && loaded.totalDeliveries == 12 && !loaded.sound &&
            loaded.difficulty == Difficulty::Arcade,
        "profile round trip preserves progress and settings");
  p.best = 1234;
  check(saveProfile(path, p) && loadProfile(path).best == 1234,
        "profile replaces an existing save");
  {
    std::ofstream bad(path);
    bad << "best -50\ndeliveries 999999999999\ndifficulty 42\n";
  }
  loaded = loadProfile(path);
  check(loaded.best == 0 && loaded.totalDeliveries == 10000000 &&
            loaded.difficulty == Difficulty::Relaxed,
        "profile values are bounded");
  std::remove(path.c_str());
  std::string unicode = folder + "/caf\xc3\xa9-\xe6\xb8\xb8\xe6\x88\x8f";
  std::filesystem::create_directories(std::filesystem::u8path(unicode));
  check(saveProfile(unicode + "/profile.dat", p) &&
            loadProfile(unicode + "/profile.dat").best == p.best,
        "Unicode profile directories work across platforms");
  std::filesystem::rename(
      std::filesystem::u8path(unicode + "/profile.dat"),
      std::filesystem::u8path(unicode + "/profile.dat.bak"));
  check(loadProfile(unicode + "/profile.dat").best == p.best,
        "interrupted replacement recovers its backup");
  std::filesystem::remove_all(std::filesystem::u8path(unicode));
  check(!saveProfile(folder + "/missing/sub/profile", p),
        "save failure is reported");
}
// A bot drives only ordinary Controls through a whole shift, including real
// traffic. No teleports, forced scores, or delivery-zone fixtures are used.
static bool bot(uint32_t seed, Difficulty difficulty) {
  Game g(seed);
  g.profile.difficulty = difficulty;
  g.start();
  std::vector<Vec> route;
  size_t index = 0;
  bool routeCarry = false;
  int routeDelivered = -1;
  int lastHearts = g.run.hearts;
  for (int frame = 0; frame < 120 * 220 && g.screen == Screen::Playing;
       ++frame) {
    bool rebuild = route.empty() || routeCarry != g.run.carrying ||
                   routeDelivered != g.run.delivered ||
                   lastHearts != g.run.hearts;
    if (rebuild) {
      routeCarry = g.run.carrying;
      routeDelivered = g.run.delivered;
      lastHearts = g.run.hearts;
      index = 0;
      if (g.run.carrying) {
        float side = g.doors[g.run.target].x < 960 ? 650.f : 1270.f;
        route = {{side, 1015}, {side, 330}, g.doors[g.run.target]};
      } else {
        float side = g.run.player.x < 960 ? 650.f : 1270.f;
        route = {{side, 330}, {side, 1015}, g.shop};
        if (g.run.player.y > 980)
          route = {g.shop};
      }
    }
    Controls c;
    if (g.canInteract())
      c.interact = true;
    else {
      while (index + 1 < route.size() &&
             length(route[index] - g.run.player) < 9)
        ++index;
      Vec delta = route[index] - g.run.player;
      float distance = length(delta);
      if (distance > 2)
        c.move = delta * (1 / distance);
      // Wait outside a lane until the projected crossing has room for a
      // courier.
      if (std::abs(c.move.y) > .7f) {
        for (const auto &car : g.run.cars) {
          float center = car.y + 22;
          float gap = (center - g.run.player.y) * c.move.y;
          if (gap > 20 && gap < 125) {
            float arrival = gap / 285;
            float future = car.x + car.speed * arrival;
            if (std::abs(future - g.run.player.x) < car.width * .5f + 75) {
              c.move = {};
              break;
            }
          }
        }
      }
    }
    g.update(1.f / 120, c);
  }
  std::printf("BOT seed=%u mode=%d deliveries=%d/%d score=%d hearts=%d "
              "remaining=%.1f won=%d\n",
              seed, int(difficulty), g.run.delivered, g.run.goal, g.run.score,
              g.run.hearts, g.run.time, g.run.won);
  return g.run.won;
}
int main(int argc, char **argv) {
  try {
    std::string suite = argc > 1 ? argv[1] : "model";
    if (suite == "model")
      model();
    else if (suite == "traffic")
      traffic();
    else if (suite == "profile")
      profile(argc > 2 ? argv[2] : ".");
    else if (suite == "playthrough" || suite == "arcade") {
      int won = 0;
      Difficulty difficulty =
          suite == "arcade" ? Difficulty::Arcade : Difficulty::Relaxed;
      for (unsigned seed = 1; seed <= 5; ++seed)
        won += bot(seed, difficulty);
      check(won >= (difficulty == Difficulty::Arcade ? 2 : 4),
            "traffic permits complete playable delivery routes across seeds "
            "without using dash");
    } else
      throw std::runtime_error("unknown suite");
    std::printf("PASS: %s (%d assertions)\n", suite.c_str(), assertions);
    return 0;
  } catch (const std::exception &e) {
    std::fprintf(stderr, "FAIL: %s\n", e.what());
    return 1;
  }
}
