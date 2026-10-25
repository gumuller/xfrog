// SPDX-License-Identifier: GPL-2.0-or-later
#include "game.hpp"
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <limits>
#include <set>
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
  for (const auto &car : g.run.cars)
    check(car.width == 170,
          "vehicle classes have consistent gameplay footprints");
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
static void animation() {
  check(courierSample(.9999f) == CourierSamples - 1 && courierSample(1.f) == 0,
        "courier loop wraps directly from final sample to first");
  check(
      std::abs(courierCycleBetween(.99f, .01f, .5f)) < .0001f,
      "render interpolation crosses the loop seam without reversing the gait");
  check(courierCycleBetween(.99f, .01f, .25f) > .99f &&
            courierCycleBetween(.99f, .01f, .75f) < .01f,
        "rendered phase advances monotonically through wraparound");
  for (int row = 0; row < 3; ++row) {
    std::set<int> keys;
    for (int key = 0; key < courierKeyCount(row); ++key) {
      int drawing = courierKey(row, key);
      check(drawing / RunFrames == row,
            "courier keeps its own directional artwork throughout the loop");
      keys.insert(drawing);
    }
    check(int(keys.size()) == courierKeyCount(row),
          "contact and passing keys do not repeat");
    check(courierKey(row, courierKeyCount(row)) == courierKey(row, 0),
          "final transition returns to the same first drawing");
  }
  for (int fps : {30, 60, 120}) {
    Game g(7);
    g.start();
    g.run.cars.clear();
    g.run.player = {100, 350};
    g.run.thief.wait = 1000;
    int previous = courierSample(g.run.animation);
    int loops = 0;
    for (int frame = 0; frame < fps * 3; ++frame) {
      for (int tick = 0; tick < 120 / fps; ++tick)
        g.update(1.f / 120, {{1, 0}, false, frame % fps == 0});
      int current = courierSample(g.run.animation);
      int step = (current - previous + CourierSamples) % CourierSamples;
      check(step <= int(std::ceil(2.5f * CourierSamples / fps)),
            "walk and dash maintain frame pacing at 30/60/120 FPS");
      check(g.run.animation >= 0 && g.run.animation < 1,
            "animation clock stays bounded without losing long-session "
            "precision");
      if (current < previous)
        ++loops;
      previous = current;
    }
    check(loops >= 4, "playback checks several actual loop boundaries");
  }
}
static void carFollowing() {
  auto car = [](float x, float y, float speed, int style = 0) {
    Car c{x, y, speed, 170, style, false};
    c.cruise = std::abs(speed);
    return c;
  };
  auto separated = [](const Game &g) {
    for (size_t i = 0; i < g.run.cars.size(); ++i)
      for (size_t j = i + 1; j < g.run.cars.size(); ++j) {
        const auto &a = g.run.cars[i], &b = g.run.cars[j];
        if (std::abs(a.y - b.y) >= 65)
          continue;
        float gap = std::abs(a.x - b.x);
        gap = std::min(gap, Width + 600 - gap);
        if (gap < 195.8f)
          return false;
      }
    return true;
  };
  Game blocked(4);
  blocked.start();
  blocked.run.player = {50, 1000};
  blocked.run.cars = {car(500, 416, 240), car(800, 416, 150),
                      car(500, 616, 150)};
  for (int i = 0; i < 120 * 20; ++i) {
    blocked.update(1.f / 120, {});
    check(separated(blocked), "blocked faster car follows without overlapping");
  }
  check(blocked.run.cars[0].y == 416 &&
            std::abs(blocked.run.cars[0].speed - 150) < 1,
        "blocked passing lane makes faster car match its leader");
  Game free(4);
  free.start();
  free.run.player = {50, 1000};
  free.run.cars = {car(500, 416, 240), car(900, 416, 150)};
  float follower = 500, leader = 900;
  bool passed = false;
  for (int i = 0; i < 120 * 20; ++i) {
    free.update(1.f / 120, {});
    follower += free.run.cars[0].speed / 120;
    leader += free.run.cars[1].speed / 120;
    check(separated(free), "overtaking car never drives through its leader");
    if (free.run.cars[0].y == 616 && follower > leader + 196)
      passed = true;
  }
  check(passed && free.run.laneChanges > 0,
        "faster car overtakes using a free same-direction lane");
  Game queue(5);
  queue.start();
  queue.run.player = {50, 1000};
  queue.run.cars = {car(1400, 516, -300), car(1160, 516, -200),
                    car(920, 516, -100), car(680, 516, -50)};
  for (int i = 0; i < 120 * 45; ++i) {
    queue.update(1.f / 120, {});
    check(separated(queue), "left-moving queues retain gaps across road wraps");
  }
  for (unsigned seed = 1; seed <= 8; ++seed) {
    Game stress(seed);
    stress.start();
    stress.start(true);
    stress.start(true);
    stress.run.player = {35, 1000};
    for (int i = 0; i < 120 * 100; ++i) {
      stress.update(1.f / 120, {});
      if (!separated(stress)) {
        std::fprintf(stderr, "OVERLAP seed=%u tick=%d\n", seed, i);
        for (const auto &c : stress.run.cars)
          std::fprintf(stderr, "x=%.2f y=%.2f v=%.2f signal=%.2f merge=%.2f\n",
                       c.x, c.y, c.speed, c.signal, c.mergeAge);
        check(false, "ordinary progressive traffic remains separated through "
                     "merges and wraps");
      }
    }
  }
}
static void progression() {
  Game g(13);
  g.start();
  float initial = std::abs(g.run.cars[0].speed);
  g.run.delivered = 2;
  ticks(g, 1);
  check(g.run.level == 2 && std::abs(g.run.cars[0].speed) > initial,
        "relaxed progression starts within the first shift");
  g.run.elapsed = 100;
  ticks(g, 1);
  check(g.run.level >= 4, "time adds pressure even without deliveries");
  g.run.player = {100, 1000};
  g.run.cars = {{600, 416, 150, 158, 0, false}};
  check(g.requestLaneChange(0),
        "clear same-direction corridor allows lane change");
  check(g.run.cars[0].signal >= 1,
        "lane change gives at least a second of warning");
  ticks(g, 120);
  check(g.run.cars[0].y == 416, "car stays in lane throughout warning");
  ticks(g, 60);
  check(g.run.cars[0].y > 416 && g.run.cars[0].y < 616,
        "lane change is continuous, not a teleport");
  ticks(g, 240);
  check(g.run.cars[0].y == 616 && g.run.cars[0].mergeAge == 0,
        "car finishes in same-direction outer lane");
  g.run.cars = {{600, 416, 150, 158, 0, false},
                {1300, 516, -178, 158, 0, false}};
  check(!g.requestLaneChange(0), "opposing traffic blocks unsafe lane changes");
  g.run.cars.resize(1);
  g.run.player = {650, 700};
  check(!g.requestLaneChange(0),
        "nearby courier prevents surprise lane changes");
  Game busy(19);
  busy.start();
  busy.start(true);
  busy.start(true);
  ticks(busy, 120 * 100);
  check(busy.run.laneChanges > 0,
        "ordinary later-shift traffic performs actual lane changes");
}
static void thief() {
  Game g(7);
  g.start();
  g.run.cars.clear();
  g.interact();
  int hearts = g.run.hearts;
  g.run.thief.wait = .001f;
  ticks(g, 1);
  check(g.run.thief.active && g.run.thief.warning > 1 &&
            g.run.thief.p.y == DeliverySidewalkY,
        "thief gives an advance entrance warning");
  g.pause();
  float warning = g.run.thief.warning;
  ticks(g, 120);
  check(g.run.thief.warning == warning, "pause freezes thief too");
  g.resume();
  auto &t = g.run.thief;
  t.warning = 0;
  g.run.player = {500, DeliverySidewalkY};
  t.p = g.run.player;
  ticks(g, 1);
  check(!g.run.carrying && t.hasPizza && g.run.stolen == 1,
        "thief steals pizza on contact");
  check(g.run.hearts == hearts && g.run.combo == 0,
        "theft breaks streak without costing a heart");
  check(std::find(g.cues.begin(), g.cues.end(), Cue::Steal) != g.cues.end(),
        "theft has its own sound cue");
  g.run.player = g.shop;
  g.interact();
  g.run.player = t.p;
  ticks(g, 1);
  check(g.run.carrying,
        "same thief cannot steal twice while carrying stolen pizza");
  t.p.x = Width + 109;
  ticks(g, 1);
  check(!t.active && t.wait >= 9, "thief escapes and has a fair respawn delay");
  check(t.direction < 0 && t.p.y == DeliverySidewalkY,
        "next pass reverses direction on delivery sidewalk");
  Game patrol(17);
  patrol.start();
  patrol.run.cars.clear();
  int appearances = 0;
  bool wasActive = false, onSidewalk = true;
  for (int i = 0; i < 120 * 150; ++i) {
    patrol.update(1.f / 120, {});
    const auto &visitor = patrol.run.thief;
    if (visitor.active && !wasActive)
      ++appearances;
    wasActive = visitor.active;
    onSidewalk = onSidewalk && visitor.p.y == DeliverySidewalkY;
  }
  check(appearances >= 3 && onSidewalk,
        "every thief appearance stays beside delivery addresses across "
        "repeated passes");
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
          if (gap > 30 && gap < 155) {
            float arrival = (gap + 40) / 285;
            float future = car.x + car.speed * arrival;
            if (std::abs(future - g.run.player.x) < car.width * .5f + 85) {
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
    else if (suite == "animation")
      animation();
    else if (suite == "following")
      carFollowing();
    else if (suite == "progression")
      progression();
    else if (suite == "thief")
      thief();
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
