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
                          416.f + lane * 100, speed, 170.f, style, false});
    }
  for (auto &car : run.cars) {
    car.cruise = std::abs(car.speed);
    car.mergeCooldown =
        std::uniform_real_distribution<float>(7, 18)(trafficRandom);
    car.previousY = car.y;
  }
  run.level = shift;
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
  floaters.push_back({{960, 152}, std::move(text), 1.6f, color});
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
    run.pickup = .32f;
    run.handoff = 0;
    run.heat = 1;
    cues.push_back(Cue::Pickup);
    toast("HOT PIZZA. GO GO GO!", Gold);
    burst(run.player, 15, Gold);
  } else {
    int tip = 20 + int(run.heat * 50) + std::min(run.combo, 5) * 15;
    run.score += 100 + tip;
    ++run.delivered;
    run.celebration = .8f;
    run.handoff = .36f;
    run.pickup = 0;
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
  run.pickup = run.handoff = 0;
  run.combo = 0;
  run.invulnerable = 2;
  run.dashTime = 0;
  run.velocity = {};
  run.player = shop;
  run.previous = shop;
  run.shake = .4f;
  cues.push_back(Cue::Hit);
  toast("OUCH! TAKE A BREATHER.", Coral);
  burst(run.player, 25, Coral);
  if (run.hearts <= 0)
    finish(false);
}
void Game::finish(bool won) {
  run.won = won;
  run.velocity = {};
  screen = Screen::Results;
  profile.best = std::max(profile.best, run.score);
  if (won)
    cues.push_back(Cue::Win);
}
void Game::update(float dt, const Controls &input) {
  if (!std::isfinite(dt) || dt <= 0)
    return;
  dt = std::min(dt, .05f);
  if (screen != Screen::Playing && screen != Screen::Results)
    return;
  updateVisuals(dt);
  if (screen == Screen::Results)
    return;
  run.previous = run.player;
  run.previousAnimation = run.animation;
  run.time = std::max(0.f, run.time - dt);
  run.elapsed += dt;
  int level =
      std::min(8, run.shift + run.delivered / 2 + int(run.elapsed / 50));
  if (level > run.level) {
    run.level = level;
    toast("TRAFFIC LEVEL " + std::to_string(level) + " - STAY SHARP!", Gold);
  }
  run.dashCooldown = std::max(0.f, run.dashCooldown - dt);
  if (run.time <= 0) {
    finish(false);
    return;
  }
  if (run.carrying) {
    run.heat -= dt / (profile.difficulty == Difficulty::Relaxed ? 45.f : 30.f);
    if (run.heat <= 0) {
      run.carrying = false;
      run.pickup = run.handoff = 0;
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
  if (length(run.velocity) > 20) {
    run.animation +=
        dt * std::min(20.f, 14.4f * length(run.velocity) / 285.f) / RunFrames;
    run.animation -= std::floor(run.animation);
  }
  if (input.interact)
    interact();
  updateTraffic(dt);
  for (auto &car : run.cars) {
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
  if (screen == Screen::Playing)
    updateThief(dt);
}
namespace {
constexpr float RoadLoop = Width + 600;
float direction(const Car &car) { return std::signbit(car.speed) ? -1.f : 1.f; }
float ahead(const Car &car, const Car &other) {
  float gap =
      std::fmod((other.x - car.x) * direction(car) + RoadLoop, RoadLoop);
  return gap < .001f ? RoadLoop : gap;
}
float clearance(const Car &a, const Car &b) {
  return (a.width + b.width) * .5f + 44;
}
bool followingLane(const Car &follower, const Car &leader) {
  if (direction(follower) != direction(leader))
    return false;
  if (std::abs(follower.y - leader.y) < 75)
    return true;
  return (leader.signal > 0 || leader.mergeAge > 0) &&
         (std::abs(follower.y - leader.mergeFrom) < 75 ||
          std::abs(follower.y - leader.mergeTo) < 75);
}
} // namespace
void Game::updateTraffic(float dt) {
  size_t count = run.cars.size();
  std::vector<float> desired(count), travel(count);
  for (size_t i = 0; i < count; ++i) {
    auto &car = run.cars[i];
    car.previousY = car.y;
    desired[i] = car.cruise > 0
                     ? car.cruise * (1 + .045f * (run.level - run.shift))
                     : std::abs(car.speed);
    car.mergeCooldown = std::max(0.f, car.mergeCooldown - dt);
  }
  for (size_t i = 0; i < count; ++i) {
    auto &car = run.cars[i];
    bool slowerAhead = false;
    for (size_t j = 0; j < count; ++j) {
      if (i == j || !followingLane(car, run.cars[j]))
        continue;
      if (ahead(car, run.cars[j]) < 520 && desired[i] > desired[j] + 12)
        slowerAhead = true;
    }
    if (car.signal > 0) {
      car.signal = std::max(0.f, car.signal - dt);
      if (car.signal == 0) {
        float planned = std::min(car.mergeSpeed, std::abs(car.speed));
        if (laneChangeClear(i, 0, planned)) {
          car.mergeAge = .0001f;
          car.mergeSpeed = planned;
        } else {
          car.mergeSpeed = 0;
          car.mergeCooldown = 1;
        }
      }
    } else if (car.mergeAge > 0) {
      car.mergeAge += dt;
      float t = std::min(1.f, car.mergeAge / 2.f);
      car.y =
          car.mergeFrom + (car.mergeTo - car.mergeFrom) * t * t * (3 - 2 * t);
      if (t >= 1) {
        car.mergeAge = 0;
        car.y = car.mergeTo;
        car.mergeCooldown =
            std::uniform_real_distribution<float>(12, 22)(trafficRandom);
      }
    } else if (slowerAhead || (run.level >= 3 && car.mergeCooldown <= 0)) {
      if (!requestLaneChange(i) && !slowerAhead)
        car.mergeCooldown = 2;
    }
  }
  // Compute following speeds from the same positions, not partially-updated
  // cars. Brake behind slower traffic while waiting for a safe overtaking gap.
  for (size_t i = 0; i < count; ++i) {
    const auto &car = run.cars[i];
    float target = desired[i];
    for (size_t j = 0; j < count; ++j) {
      if (i == j || !followingLane(car, run.cars[j]))
        continue;
      const auto &leader = run.cars[j];
      float gap = ahead(car, leader);
      target = std::min(
          target,
          std::max(0.f, std::abs(leader.speed) +
                            (gap - clearance(car, leader) - 30.f) / .85f));
    }
    if ((car.signal > 0 || car.mergeAge > 0) && car.mergeSpeed > 0)
      target = std::min(target, car.mergeSpeed);
    float speed = std::abs(car.speed);
    float acceleration = target > speed ? 95.f : 400.f;
    speed += std::clamp(target - speed, -acceleration * dt, acceleration * dt);
    travel[i] = speed * dt;
  }
  // Enforce a bumper gap for whole queues, including both sides of a wrap and
  // lanes reserved by merging vehicles. Constraint relaxation handles chains.
  for (size_t pass = 0; pass < count; ++pass) {
    bool changed = false;
    for (size_t i = 0; i < count; ++i)
      for (size_t j = 0; j < count; ++j) {
        if (i == j || !followingLane(run.cars[i], run.cars[j]))
          continue;
        float limit =
            std::max(0.f, ahead(run.cars[i], run.cars[j]) -
                              clearance(run.cars[i], run.cars[j]) + travel[j]);
        if (travel[i] > limit) {
          travel[i] = limit;
          changed = true;
        }
      }
    if (!changed)
      break;
  }
  for (size_t i = 0; i < count; ++i) {
    auto &car = run.cars[i];
    float sign = direction(car);
    car.speed = std::copysign(travel[i] / dt, sign);
    car.x += sign * travel[i];
    if (car.x > Width + 300) {
      car.x -= RoadLoop;
      car.nearAwarded = false;
    }
    if (car.x < -300) {
      car.x += RoadLoop;
      car.nearAwarded = false;
    }
  }
}
bool Game::requestLaneChange(size_t index) {
  if (index >= run.cars.size())
    return false;
  auto &car = run.cars[index];
  // Only same-direction outer lanes exchange traffic. The opposing middle
  // lane must have a clear projected corridor for the whole maneuver.
  if (car.speed <= 0 || car.signal > 0 || car.mergeAge > 0 || car.x < 220 ||
      car.x > Width - 220 || std::abs(car.x - run.player.x) < 260 ||
      (std::abs(car.y - 416) > 1 && std::abs(car.y - 616) > 1))
    return false;
  float planned = std::abs(car.speed);
  for (size_t j = 0; j < run.cars.size(); ++j) {
    if (j != index && followingLane(car, run.cars[j]) &&
        ahead(car, run.cars[j]) < 650)
      planned = std::min(planned, std::abs(run.cars[j].speed));
  }
  if (planned < 40 || !laneChangeClear(index, 1.2f, planned))
    return false;
  car.mergeSpeed = planned;
  ++run.laneChanges;
  car.mergeFrom = car.y;
  car.mergeTo = car.y < 500 ? 616.f : 416.f;
  car.signal = 1.2f;
  car.mergeAge = 0;
  return true;
}
bool Game::laneChangeClear(size_t index, float delay, float speed) const {
  const auto &car = run.cars[index];
  float target = car.y < 500 ? 616.f : 416.f;
  auto inverse = [](float value) {
    float lo = 0, hi = 1;
    for (int i = 0; i < 22; ++i) {
      float mid = (lo + hi) * .5f;
      if (mid * mid * (3 - 2 * mid) < value)
        lo = mid;
      else
        hi = mid;
    }
    return (lo + hi) * .5f;
  };
  auto distance = [](double delta) {
    double d = std::fmod(std::abs(delta), double(RoadLoop));
    return std::min(d, double(RoadLoop) - d);
  };
  for (size_t j = 0; j < run.cars.size(); ++j) {
    if (j == index)
      continue;
    const auto &other = run.cars[j];
    if (other.signal > 0 || other.mergeAge > 0)
      return false;
    float low = (other.y - 65 - car.y) / (target - car.y),
          high = (other.y + 65 - car.y) / (target - car.y);
    if (low > high)
      std::swap(low, high);
    if (high < 0 || low > 1)
      continue;
    // Exact time interval of lateral overlap under smoothstep steering.
    // Test the entire swept interval, including screen wraps, rather than
    // sparse samples that can miss two cars crossing between frames.
    double begin =
        low <= 0 ? 0 : delay + 2 * inverse(std::clamp(low, 0.f, 1.f));
    double end =
        high >= 1 ? delay + 2 : delay + 2 * inverse(std::clamp(high, 0.f, 1.f));
    double relative = direction(car) * speed - other.speed;
    double first = car.x - other.x + relative * begin,
           last = car.x - other.x + relative * end;
    double lo = std::min(first, last), hi = std::max(first, last);
    double nearest = std::ceil(lo / RoadLoop) * RoadLoop;
    double gap = nearest <= hi ? 0 : std::min(distance(first), distance(last));
    if (gap < (car.width + other.width) * .5f + 45)
      return false;
  }
  return true;
}
void Game::updateThief(float dt) {
  auto &thief = run.thief;
  thief.previous = thief.p;
  if (!thief.active) {
    thief.wait -= dt;
    if (thief.wait > 0)
      return;
    thief.active = true;
    thief.hasPizza = false;
    thief.warning = 1.5f;
    thief.p = {thief.direction > 0 ? -95.f : Width + 95, DeliverySidewalkY};
    thief.previous = thief.p;
    toast("PIZZA THIEF INCOMING - KEEP YOUR DISTANCE!", Coral);
  }
  if (thief.warning > 0) {
    thief.warning = std::max(0.f, thief.warning - dt);
    return;
  }
  float speed = (thief.hasPizza ? 245.f : 150.f) + 8 * run.level;
  thief.p.x += thief.direction * speed * dt;
  thief.animation += dt * std::max(1.8f, speed / 150.f);
  if (!thief.hasPizza && run.carrying && run.invulnerable <= 0 &&
      length(run.player - thief.p) < 38) {
    thief.hasPizza = true;
    run.carrying = false;
    run.pickup = run.handoff = 0;
    run.combo = 0;
    ++run.stolen;
    cues.push_back(Cue::Steal);
    toast("THE THIEF TOOK YOUR PIZZA! GRAB A FRESH ONE.", Coral);
    burst(run.player, 18, Coral);
  }
  if (thief.p.x > Width + 110 || thief.p.x < -110) {
    thief.active = false;
    thief.direction *= -1;
    thief.p.y = DeliverySidewalkY;
    thief.wait = std::max(9.f, 21.f - run.level * 1.2f);
  }
}
void Game::updateVisuals(float dt) {
  run.invulnerable = std::max(0.f, run.invulnerable - dt);
  run.shake = std::max(0.f, run.shake - dt);
  run.celebration = std::max(0.f, run.celebration - dt);
  run.pickup = std::max(0.f, run.pickup - dt);
  run.handoff = std::max(0.f, run.handoff - dt);
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
    f.p.y -= 5 * dt;
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
