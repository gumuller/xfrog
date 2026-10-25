// SPDX-License-Identifier: GPL-2.0-or-later
#include "game.hpp"
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <sstream>
namespace xfrog {
constexpr uint32_t Mint = 0x94efd0, Gold = 0xffd47b, Coral = 0xff8e80;
float Game::randomFloat(float lo, float hi) {
  return std::uniform_real_distribution<float>(lo, hi)(random);
}
void Game::start(bool next) {
  int shift = next ? run.shift + 1 : 1;
  run = Session{};
  run.shift = shift;
  run.goal = std::min(6 + (shift - 1) * 2, 14);
  run.time = profile.difficulty == Difficulty::Relaxed ? 180.f : 150.f;
  run.hearts = profile.difficulty == Difficulty::Relaxed ? 5 : 3;
  int count = profile.difficulty == Difficulty::Relaxed ? 3 : 4;
  for (int lane = 0; lane < 3; ++lane)
    for (int i = 0; i < count; ++i) {
      int style = (lane + i) % 4;
      float speed = (lane == 1 ? -1.f : 1.f) *
                    (150 + lane * 28 + std::min(shift - 1, 8) * 12);
      run.cars.push_back({-260.f + i * (2440.f / count) + (lane % 2) * 230,
                          416.f + lane * 100, speed, style < 2 ? 158.f : 205.f,
                          style, false});
    }
  particles.clear();
  floaters.clear();
  cues.clear();
  screen = Screen::Playing;
  toast("FRESH SHIFT. LET'S ROLL!", Mint);
}
void Game::pause() {
  if (screen == Screen::Playing) {
    screen = Screen::Paused;
    run.velocity = {};
  }
}
void Game::resume() {
  if (screen == Screen::Paused)
    screen = Screen::Playing;
}
void Game::menu() {
  screen = Screen::Menu;
  run.velocity = {};
}
void Game::help() {
  helpReturn = screen;
  screen = Screen::Help;
  run.velocity = {};
}
void Game::closeHelp() { screen = helpReturn; }
bool Game::clearAt(Vec p) const {
  return p.x >= 35 && p.x <= Width - 35 && p.y >= 320 && p.y <= Height - 28 &&
         !shopSolid.contains(p, 14);
}
bool Game::canInteract() const {
  return screen == Screen::Playing &&
         length(run.player - (run.carrying ? doors[run.target] : shop)) < 85;
}
void Game::toast(std::string text, uint32_t color) {
  floaters.clear();
  floaters.push_back({{960, 180}, std::move(text), 1.6f, color});
}
void Game::burst(Vec p, int count, uint32_t color) {
  for (int i = 0; i < count && particles.size() < 180; ++i) {
    float a = randomFloat(0, 6.283185f), s = randomFloat(35, 180),
          t = randomFloat(.35f, .85f);
    particles.push_back({p,
                         {std::cos(a) * s, std::sin(a) * s},
                         t,
                         t,
                         randomFloat(3, 7),
                         color});
  }
}
void Game::interact() {
  if (!canInteract())
    return;
  if (!run.carrying) {
    int last = run.target;
    run.target = std::uniform_int_distribution<int>(0, 3)(random);
    if (run.target >= last)
      ++run.target;
    run.carrying = true;
    run.heat = 1;
    cues.push_back(Cue::Pickup);
    toast("HOT PIZZA. GO GO GO!", Gold);
    burst(run.player, 15, Gold);
  } else {
    int tip = 20 + int(run.heat * 50) + std::min(run.combo, 5) * 15;
    run.score += 100 + tip;
    ++run.delivered;
    run.celebration = .8f;
    ++run.combo;
    run.bestCombo = std::max(run.bestCombo, run.combo);
    run.carrying = false;
    ++profile.totalDeliveries;
    profile.best = std::max(profile.best, run.score);
    cues.push_back(Cue::Deliver);
    toast("DELIVERED! +" + std::to_string(100 + tip), Gold);
    burst(run.player, 45, Gold);
    if (run.delivered >= run.goal)
      finish(true);
  }
}
void Game::hit() {
  if (screen != Screen::Playing || run.invulnerable > 0)
    return;
  --run.hearts;
  run.carrying = false;
  run.combo = 0;
  run.invulnerable = 2;
  run.dashTime = 0;
  run.velocity = {};
  run.player = shop;
  run.shake = .4f;
  cues.push_back(Cue::Hit);
  toast("OUCH! TAKE A BREATHER.", Coral);
  burst(run.player, 25, Coral);
  if (run.hearts <= 0)
    finish(false);
}
void Game::finish(bool won) {
  run.won = won;
  screen = Screen::Results;
  profile.best = std::max(profile.best, run.score);
  if (won)
    cues.push_back(Cue::Win);
}
void Game::update(float dt, const Controls &input) {
  if (!std::isfinite(dt) || dt <= 0)
    return;
  dt = std::min(dt, .05f);
  if (screen != Screen::Playing)
    return;
  run.previous = run.player;
  run.time = std::max(0.f, run.time - dt);
  run.elapsed += dt;
  run.invulnerable = std::max(0.f, run.invulnerable - dt);
  run.shake = std::max(0.f, run.shake - dt);
  run.celebration = std::max(0.f, run.celebration - dt);
  run.dashCooldown = std::max(0.f, run.dashCooldown - dt);
  if (run.time <= 0) {
    finish(false);
    return;
  }
  if (run.carrying) {
    run.heat -= dt / (profile.difficulty == Difficulty::Relaxed ? 45.f : 30.f);
    if (run.heat <= 0) {
      run.carrying = false;
      run.combo = 0;
      run.score = std::max(0, run.score - 25);
      cues.push_back(Cue::Expired);
      toast("PIZZA WENT COLD. GRAB ANOTHER!", Coral);
    }
  }
  Vec direction = input.move;
  float magnitude = length(direction);
  if (magnitude > 1)
    direction = direction * (1 / magnitude);
  if (length(direction) > .05f) {
    run.facing = std::abs(direction.x) > std::abs(direction.y)
                     ? (direction.x > 0 ? 1 : 3)
                     : (direction.y > 0 ? 0 : 2);
    if (input.dash && run.dashCooldown <= 0) {
      run.dashTime = .19f;
      run.dashCooldown = 1.65f;
      cues.push_back(Cue::Dash);
      burst(run.player, 9, Mint);
    }
  }
  run.dashTime = std::max(0.f, run.dashTime - dt);
  Vec desired = direction * (run.dashTime > 0 ? 700.f : 285.f);
  // Exponential approach is independent of frame rate; full speed in a few
  // frames.
  float blend = 1 - std::exp(-24 * dt);
  run.velocity = run.velocity + (desired - run.velocity) * blend;
  Vec next = run.player + run.velocity * dt;
  if (clearAt({next.x, run.player.y}))
    run.player.x = next.x;
  else
    run.velocity.x = 0;
  if (clearAt({run.player.x, next.y}))
    run.player.y = next.y;
  else
    run.velocity.y = 0;
  if (length(run.velocity) > 20)
    run.animation += dt * (run.dashTime > 0 ? 22.f : 12.f);
  else
    run.animation = 0;
  if (input.interact)
    interact();
  for (auto &car : run.cars) {
    car.x += car.speed * dt;
    if (car.x > Width + 300) {
      car.x = -300;
      car.nearAwarded = false;
    }
    if (car.x < -300) {
      car.x = Width + 300;
      car.nearAwarded = false;
    }
    Rect box{car.x - car.width * .43f, car.y + 5, car.width * .86f, 35};
    if (box.contains(run.player, 13)) {
      hit();
      break;
    }
    if (!car.nearAwarded && run.invulnerable <= 0 &&
        std::abs(run.player.x - car.x) < car.width * .45f &&
        std::abs(run.player.y - (car.y + 22)) > 50 &&
        std::abs(run.player.y - (car.y + 22)) < 70) {
      car.nearAwarded = true;
      run.score += 5;
      profile.best = std::max(profile.best, run.score);
      ++run.nearMisses;
      toast("CLOSE CALL +5", Mint);
      cues.push_back(Cue::NearMiss);
    }
  }
  for (auto &p : particles) {
    p.life -= dt;
    p.p = p.p + p.v * dt;
    p.v.y += 90 * dt;
  }
  particles.erase(std::remove_if(particles.begin(), particles.end(),
                                 [](const Particle &p) { return p.life <= 0; }),
                  particles.end());
  for (auto &f : floaters) {
    f.life -= dt;
    f.p.y -= 25 * dt;
  }
  floaters.erase(std::remove_if(floaters.begin(), floaters.end(),
                                [](const Floater &f) { return f.life <= 0; }),
                 floaters.end());
}
Profile loadProfile(const std::string &path) {
  Profile p;
  std::ifstream file(std::filesystem::u8path(path));
  if (!file)
    file.open(std::filesystem::u8path(path + ".bak"));
  std::string key;
  long long value;
  while (file >> key >> value) {
    if (key == "best")
      p.best = int(std::clamp(value, 0ll, 100000000ll));
    else if (key == "deliveries")
      p.totalDeliveries = int(std::clamp(value, 0ll, 10000000ll));
    else if (key == "sound")
      p.sound = value != 0;
    else if (key == "difficulty")
      p.difficulty = value == 1 ? Difficulty::Arcade : Difficulty::Relaxed;
  }
  return p;
}
bool saveProfile(const std::string &path, const Profile &p) {
  namespace fs = std::filesystem;
  fs::path destination = fs::u8path(path),
           temporary = fs::u8path(path + ".tmp");
  {
    std::ofstream out(temporary, std::ios::trunc);
    out << "best " << p.best << "\ndeliveries " << p.totalDeliveries
        << "\nsound " << p.sound << "\ndifficulty " << int(p.difficulty)
        << "\n";
    out.flush();
    if (!out)
      return false;
  }
  std::error_code error;
#ifdef _WIN32
  // Preserve the old profile until the replacement has been moved into place.
  fs::path backup = fs::u8path(path + ".bak");
  fs::remove(backup, error);
  error.clear();
  bool existed = fs::exists(destination, error);
  if (error)
    return false;
  if (existed) {
    fs::rename(destination, backup, error);
    if (error)
      return false;
  }
  fs::rename(temporary, destination, error);
  if (error) {
    if (existed) {
      error.clear();
      fs::rename(backup, destination, error);
    }
    return false;
  }
  fs::remove(backup, error);
  return true;
#else
  fs::rename(temporary, destination, error);
  return !error;
#endif
}
} // namespace xfrog
