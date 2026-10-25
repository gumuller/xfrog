// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <random>
#include <string>
#include <vector>
namespace xfrog {
constexpr float Width = 1920, Height = 1080;
constexpr const char *Version = "2.0.0";
struct Vec {
  float x = 0, y = 0;
  Vec operator+(Vec b) const { return {x + b.x, y + b.y}; }
  Vec operator-(Vec b) const { return {x - b.x, y - b.y}; }
  Vec operator*(float s) const { return {x * s, y * s}; }
};
inline float length(Vec v) { return std::sqrt(v.x * v.x + v.y * v.y); }
struct Rect {
  float x, y, w, h;
  bool contains(Vec p, float pad = 0) const {
    return p.x > x - pad && p.x < x + w + pad && p.y > y - pad &&
           p.y < y + h + pad;
  }
};
enum class Difficulty { Relaxed, Arcade };
enum class Screen { Menu, Playing, Paused, Results, Help };
enum class Cue { Pickup, Deliver, Hit, Dash, NearMiss, Expired, Win, Click };
struct Controls {
  Vec move;
  bool interact = false, dash = false;
};
struct Car {
  float x = 0, y = 0, speed = 0, width = 0;
  int style = 0;
  bool nearAwarded = false;
};
struct Particle {
  Vec p, v;
  float life = 0, total = 0, size = 0;
  uint32_t color = 0;
};
struct Floater {
  Vec p;
  std::string text;
  float life = 1.5f;
  uint32_t color = 0;
};
struct Profile {
  int best = 0, totalDeliveries = 0;
  bool sound = true;
  Difficulty difficulty = Difficulty::Relaxed;
};
struct Session {
  Vec player{960, 1015}, velocity{}, previous{960, 1015};
  float time = 150, elapsed = 0, heat = 1, invulnerable = 0, dashTime = 0,
        dashCooldown = 0, animation = 0, shake = 0, celebration = 0, pickup = 0,
        handoff = 0;
  int facing = 0, score = 0, delivered = 0, combo = 0, bestCombo = 0,
      hearts = 3, target = 0, shift = 1, goal = 6, nearMisses = 0;
  bool carrying = false, won = false;
  std::vector<Car> cars;
};
class Game {
public:
  Screen screen = Screen::Menu, helpReturn = Screen::Menu;
  Profile profile;
  Session run;
  std::vector<Particle> particles;
  std::vector<Floater> floaters;
  std::vector<Cue> cues;
  const Vec shop{960, 1010};
  const Vec doors[5] = {
      {300, 330}, {620, 330}, {963, 330}, {1280, 330}, {1610, 330}};
  const char *shops[5] = {"SUNRISE BAKERY", "THE GREEN GROCER", "HOTEL AZURE",
                          "THE WORKSHOP", "CORAL CAFE"};
  const Rect shopSolid{690, 685, 530, 285};
  explicit Game(uint32_t seed = 1) : random(seed) {}
  void start(bool next = false);
  void update(float dt, const Controls &input);
  void pause();
  void resume();
  void menu();
  void help();
  void closeHelp();
  bool canInteract() const;
  void interact();
  void hit();
  void burst(Vec p, int count, uint32_t color);
  bool clearAt(Vec p) const;

private:
  std::mt19937 random;
  float randomFloat(float lo, float hi);
  void toast(std::string text, uint32_t color);
  void finish(bool won);
  void updateVisuals(float dt);
};
Profile loadProfile(const std::string &path);
bool saveProfile(const std::string &path, const Profile &profile);
} // namespace xfrog
